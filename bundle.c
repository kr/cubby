// bundle.c -- A disk file that contains cubby data

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "bundle.h"
#include "region.h"
#include "util.h"

#define BUNDLE_MAGIC 0x7ade2ae6

int initialize_bundles = 0;

/* returns 0 on success, -1 on error */
int
add_bundle(manager mgr, char *name)
{
    struct bundle *new_bundles;

    new_bundles = realloc(mgr->all_bundles,
                          sizeof(struct bundle) * (mgr->nbundles + 1));
    if (!new_bundles) return warn("realloc"), -1;

    mgr->all_bundles = new_bundles;

    mgr->all_bundles[mgr->nbundles].name = name;
    mgr->all_bundles[mgr->nbundles].manager = mgr;
    mgr->all_bundles[mgr->nbundles].tot_size = 0;
    mgr->all_bundles[mgr->nbundles].reg_size = 0;
    mgr->all_bundles[mgr->nbundles].storage = 0;
    mgr->all_bundles[mgr->nbundles].nregions = 0;

    ++mgr->nbundles;
    return 0;
}

bundle
bundle_get(manager mgr, uint16_t i)
{
    return &mgr->all_bundles[i];
}

void
bundle_sync(bundle b, int sync)
{
    int r;
    r = msync(b->storage, b->tot_size, sync ? MS_SYNC : MS_ASYNC);
    if (r == -1) warn("msync");
}

region_storage
bundle_get_region_storage(bundle b, uint16_t i)
{
    size_t off = ((size_t) i) << REGION_BITS;
    return (region_storage) (b->storage->regions + off);
}

int
bundle_make_root_key(uint32_t *key)
{
    char *buf = (char *) key;

    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) return warn("open(/dev/urandom)"), -1;

    int n = 0;
    while (n < 12) {
        int r = read(fd, buf + n, 12 - n);
        if (r == -1) return warn("read"), -1;
        n += r;
    }

    for (int r = -1; r == -1; ) {
        r = close(fd);
        if (r == -1) {
            if (errno == EBADF) return 0;
            if (errno == EIO) return -1;
        }
    }
    return 0;
}

static int
bundle_init(bundle b)
{
    uint16_t i;
    int r;

    memset(b->storage, 0, sizeof(struct bundle_storage));

    for (i = 0; i < b->nregions; i++) {
        region_storage rs = bundle_get_region_storage(b, i);

        // write region header
        rs->flags = 0;
    }

    // write bundle header
    b->storage->magic = BUNDLE_MAGIC;
    b->storage->version = BUNDLE_VERSION;
    r = bundle_make_root_key(b->storage->root_key);
    if (r == -1) return warnx("could not make root key for %s", b->name), -1;

    bundle_sync(b, 1);

    return 0;
}

int
bundle_open(bundle b)
{
    int fd, r;

    fd = open(b->name, O_RDWR|O_CREAT, 0600);
    if (fd == -1) return warn("open(%s)", b->name), -1;

    r = fcntl(fd, F_SETFD, FD_CLOEXEC);
    if (r == -1) {
        warn("fcntl");
        close(fd);
        return -1;
    }

    b->tot_size = lseek(fd, 0, SEEK_END);
    warnx("size of %s is %lld bytes", b->name, b->tot_size);
    lseek(fd, 0, SEEK_SET); // Put it back. (Is this necessary?)

    b->reg_size = b->tot_size - sizeof(struct bundle_storage);
    b->nregions = ((b->reg_size - 1) >> REGION_BITS) + 1;

    b->storage = mmap(0, b->tot_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (b->storage == MAP_FAILED) {
        warn("mmap");
        close(fd);
        return -1;
    }

    r = close(fd);
    if (r == -1) {
        warn("close");
        munmap(b->storage, b->tot_size);
        return -1;
    }

    if (b->storage->magic != BUNDLE_MAGIC) {
        if (!initialize_bundles) {
            munmap(b->storage, b->tot_size);
            return warnx("bundle magic number mismatch"), -1;
        }

        r = bundle_init(b);
        if (r == -1) return warnx("failed to init"), -1;
    }
    if (b->storage->version != BUNDLE_VERSION) {
        munmap(b->storage, b->tot_size);
        return warnx("bundle version mismatch"), -1;
    }
    return 0;
}

