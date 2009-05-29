// bundle.c -- A disk file that contains cubby data

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "bundle.h"
#include "util.h"

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
    all_bundles[nbundles].base = 0;
    all_bundles[nbundles].nregions = 0;

    ++nbundles;
    return 0;
}

static int
bundle_init(bundle b)
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
    b->size = stat_s.st_size;

    b->base = mmap(0, b->size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (b->base == MAP_FAILED) {
        warn("mmap");
        close(fd);
        return -1;
    }

    warnx("woo! got a bundle");
    // read header -- init if necessary

    return 0;
}

int
bundles_init()
{
    int i, r;
    size_t nregions = 0;

    if (nbundles < 1) return raw_warnx("no bundles defined"), -1;

    for (i = 0; i < nbundles; i++) {
        bundle b = all_bundles + i;
        r = bundle_init(b);
        if (r == -1) {
            warnx("error with bundle %s\n", b->name);
            // TODO do something sensible here
        } else {
            nregions += b->nregions;
        }
    }
    return 0;
}
