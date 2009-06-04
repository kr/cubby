// region.c -- Contiguous space inside a bundle

#include <stdlib.h>

#include "blob.h"
#include "region.h"
#include "bundle.h"
#include "dirent.h"
#include "spht.h"
#include "util.h"

size_t
region_blob_offset(region r, blob b)
{
    return ((char *) b) - r->storage->blobs;
}

blob
region_get_blob_by_off(region r, uint32_t off)
{
    return (blob) (r->storage->blobs + off);
}

int
region_has_space_for_blob(region r, size_t size)
{
    char *next_free;
    blob new = (blob) r->free;

    // Find the end of this blob's space.
    next_free = ALIGN(new->data + size, char *);

    return next_free <= r->top;
}

blob
region_allocate_blob(region r, size_t size)
{
    blob new;
    char *next_free;

    new = (blob) r->free;

    // Find the end of this blob's space.
    next_free = ALIGN(new->data + size, char *);
    if (next_free > r->top) return 0;

    r->free = next_free; // Actually allocate the space.

    return new;
}

void
region_delete_blob(region r, blob b)
{
    warnx("STUB");
    exit(66);
}

static int
region_close_to_full(region r)
{
    return 0;
}

int
regions_init(manager mgr, uint16_t count)
{
    int nbundles, i, j, n = 0;

    mgr->all_regions = malloc(sizeof(struct region) * count);
    if (!mgr->all_regions) return warn("malloc"), -1;
    mgr->nregions = count;

    nbundles = bundles_count();
    for (i = 0; i < nbundles; i++) {
        bundle bun = bundle_get(mgr, i);
        for (j = 0; j < bun->nregions; j++) {
            blob bl;
            size_t reg_total_size, reg_total_cap;
            region reg = &mgr->all_regions[n++];
            reg->id = n - 1;

            // Total size of the region, including the headers
            reg_total_size = 1 << REGION_BITS;
            if (j == bun->nregions - 1) { // the last region might be shorter
                reg_total_size = bun->reg_size % reg_total_size;
            }

            // Capacity of the region, not including the headers
            reg_total_cap = reg_total_size - sizeof(struct region);

            reg->storage = bundle_get_region_storage(bun, j);
            reg->top = ((char *) reg->storage) + reg_total_cap;

            for (bl = (blob) reg->storage->blobs; ; bl = blob_next(bl)) {
                int r;
                dirent de;
                rdesc_local rdesc;

                if (!bl->size) break;
                raw_warnx("blob size is %d", bl->size);

                /* This blob claims to extend past the end of the region! */
                if (((char *) bl) >= reg->top) {
                    warnx("%s: last blob overextended", bun->name);
                    break;
                }

                r = blob_verify(bl);
                if (r == -1) {
                    warnx("verification failed for blob at offset 0x%x\n",
                          region_blob_offset(reg, bl));
                    continue;
                }

                de = make_dirent(bl->key, 1);
                rdesc = (rdesc_local) &de->rdescs[0];
                rdesc->flags = RDESC_LOCAL;
                rdesc->reg = reg->id;
                rdesc->off = region_blob_offset(reg, bl);
                spht_set(mgr->directory, de);
                raw_warnx("read one k = %8x.%8x.%8x", bl->key[2], bl->key[1], bl->key[0]);
            }
            reg->free = (char *) bl;
            if (!region_close_to_full(reg)) {
                manager_add_free_region(mgr, reg);
            }
        }
    }

    return 0;
}
