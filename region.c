// region.c -- Contiguous space inside a bundle

#include <stdlib.h>

#include "blob.h"
#include "region.h"
#include "bundle.h"
#include "dirent.h"
#include "spht.h"
#include "key.h"
#include "util.h"

#define REGION_MAGIC 0x8c9ba2d0

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

static void
mark_blob_sentinel(char *free)
{
    ((blob) free)->size = 0;
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
    mark_blob_sentinel(next_free);

    return new;
}

void
region_delete_blob(region r, blob b)
{
    warnx("STUB");
    exit(66);
}

void
region_init(region r, int id, region_storage storage, size_t size)
{
    r->id = id;
    r->storage = storage;
    r->free = r->storage->blobs;
    r->top = ((char *) r->storage) + size;
    r->next = 0;
}

static void
region_init_storage(region reg)
{
    reg->storage->magic = REGION_MAGIC;
    mark_blob_sentinel(reg->storage->blobs);
}

void
region_read(region reg, manager mgr, const char *bundle_name)
{
    if (reg->storage->magic != REGION_MAGIC) {
        return region_init_storage(reg);
    }

    blob bl;
    for (bl = (blob) reg->storage->blobs; ; bl = blob_next(bl)) {
        int r;
        dirent de;
        rdesc_local rdesc;

        if (!bl->size) break;
        raw_warnx("blob size is %d", bl->size);

        /* This blob claims to extend past the end of the region! */
        if (((char *) bl) >= reg->top) {
            warnx("%s: last blob overextended", bundle_name);
            break;
        }

        r = blob_verify(bl);
        if (r == -1) {
            warnx("verification failed for blob at offset 0x%x\n",
                    region_blob_offset(reg, bl));
            continue;
        }

        de = make_dirent(bl->key, 1, 3);
        rdesc = (rdesc_local) &de->rdescs[0];
        rdesc->flags = RDESC_LOCAL;
        rdesc->reg = reg->id;
        rdesc->off = region_blob_offset(reg, bl);
        spht_set(mgr->directory, de);
        char fkey[27];
        key_fmt(fkey, bl->key);
        raw_warnx("read one k = %s", fkey);
    }
    reg->free = (char *) bl;
}

static size_t
region_free_space(region r)
{
    return r->top - r->free;
}

int
region_space_cmp(region a, region b)
{
    size_t sa, sb;

    sa = region_free_space(a);
    sb = region_free_space(b);

    if (sa > sb) return 1;
    if (sa < sb) return -1;

    if (a->id > b->id) return 1;
    if (a->id < b->id) return -1;

    return 0;
}

