// region.c -- Contiguous space inside a bundle

#include <stdlib.h>

#include "blob.h"
#include "region.h"
#include "bundle.h"
#include "util.h"

static struct region *all_regions = 0;
static int nregions = 0;

int
regions_init(uint16_t count)
{
    int nbundles, i, j, n = 0;

    all_regions = malloc(sizeof(struct region) * count);
    if (!all_regions) return warn("malloc"), -1;
    nregions = count;

    nbundles = bundles_count();
    for (i = 0; i < nbundles; i++) {
        bundle bun = bundle_get(i);
        for (j = 0; j < bun->nregions; j++) {
            blob bl;
            region reg = &all_regions[n++];
            reg->size = 1 << REGION_BITS;
            if (j == bun->nregions - 1) { // last region
                reg->size = bun->reg_size % reg->size; // might be shorter
            }
            reg->storage = bundle_get_region_storage(bun, j);

            blob rtop = (blob) (((char *) reg->storage) + reg->size);

            for (bl = (blob) reg->storage->blobs; bl < rtop; bl = blob_next(bl)) {
                int r;

                if (!bl->size) break;
                raw_warnx("blob size is %d", bl->size);

                r = blob_verify(bl);
                if (r == -1) {
                    warnx("verification failed for blob at offset 0x%x\n",
                          ((char *) bl) - ((char *) reg->storage));
                    continue;
                }
                // insert into the directory
            }
        }
    }

    return 0;
}
