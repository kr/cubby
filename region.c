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

void
region_init(region r, int id, region_storage storage, size_t size)
{
    r->id = id;
    r->storage = storage;
    r->free = r->storage->blobs;
    r->top = ((char *) storage) + size;
    r->next = 0;
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

