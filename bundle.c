// bundle.c -- A disk file that contains cubby data

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#include "bundle.h"
#include "region.h"
#include "util.h"

#define BUNDLE_MAGIC 0x7ade2ae6

int initialize_bundles = 0;

// This points to an array of structs, not a single one.
static struct bundle *all_bundles = 0;
static int nbundles = 0;

/* returns 0 on success, -1 on error */
int
add_bundle(char *name)
{
    struct bundle *new_bundles;

    new_bundles = realloc(all_bundles, sizeof(struct bundle) * (nbundles + 1));
    if (!new_bundles) return warn("realloc"), -1;

    all_bundles = new_bundles;

    all_bundles[nbundles].name = name;
    all_bundles[nbundles].tot_size = 0;
    all_bundles[nbundles].reg_size = 0;
    all_bundles[nbundles].storage = 0;
    all_bundles[nbundles].nregions = 0;

    ++nbundles;
    return 0;
}

uint16_t
bundles_count()
{
    return nbundles;
}

bundle
bundle_get(uint16_t i)
{
    return &all_bundles[i];
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
    return (region_storage) (b->storage->regions + (i << REGION_BITS));
}

static int
bundle_init(bundle b)
{
    uint16_t i;

    memset(b->storage, 0, b->tot_size);

    for (i = 0; i < b->nregions; i++) {
        region_storage rs = bundle_get_region_storage(b, i);

        // write region header
        rs->flags = 0;
    }

    // write bundle header
    b->storage->magic = BUNDLE_MAGIC;
    b->storage->version = BUNDLE_VERSION;

    bundle_sync(b, 1);

    return 0;
}

static int
bundle_open(bundle b)
{
    int fd, r;
    struct stat stat_s;

    fd = open(b->name, O_RDWR|O_CREAT, 0600);
    if (fd == -1) return warn("open(%s)", b->name), -1;

    r = fcntl(fd, F_SETFD, FD_CLOEXEC);
    if (r == -1) {
        warn("fcntl");
        close(fd);
        return -1;
    }

    r = fstat(fd, &stat_s);
    if (r == -1) {
        warn("fstat");
        close(fd);
        return -1;
    }
    b->tot_size = stat_s.st_size;
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

int
bundles_init(spht directory)
{
    int i, r;
    size_t nregions = 0;

    if (nbundles < 1) return raw_warnx("no bundles defined"), -1;

    for (i = 0; i < nbundles; i++) {
        bundle b = all_bundles + i;
        r = bundle_open(b);
        if (r == -1) {
            warnx("error with bundle %s; skipping", b->name);
            continue;
        }
        nregions += b->nregions;
    }

    if (nregions < 1) return warnx("no valid regions"), -2;

    r = regions_init(nregions, directory);
    if (r == -1) return warnx("regions_init"), -2;

    return 0;
}

