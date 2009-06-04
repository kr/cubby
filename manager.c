#include <string.h>

#include "bundle.h"
#include "manager.h"
#include "util.h"

void
manager_add_free_region(manager m, region r)
{
    if (!m->free_regions_head) m->free_regions_head = r;
    if (m->free_regions_tail) m->free_regions_tail->next = r;
    m->free_regions_tail = r;
    r->next = 0;
}

region
manager_pick_region(manager m, size_t size)
{
    region r;

    r = m->free_regions_head;
    while (r && !region_has_space_for_blob(r, size)) {
        m->free_regions_head = r = r->next;
    }
    return r;
}

blob
manager_allocate_blob(manager m, dirent d, size_t size)
{
    region r;
    blob b;

    r = manager_pick_region(m, size);
    if (!r) return 0;

    raw_warnx("allocating blob in region %d\n", r->id);

    b = region_allocate_blob(r, size);
    if (!b) return 0;

    dirent_set_rdesc_local(d, 0, r, b);

    return b;
}

region
manager_get_region(manager m, dirent d)
{
    rdesc_local rd;

    rd = dirent_get_rdesc_local(d);
    return &m->all_regions[rd->reg];
}

blob
manager_get_blob(manager m, dirent d)
{
    rdesc_local rd;

    rd = dirent_get_rdesc_local(d);
    return region_get_blob_by_off(manager_get_region(m, d), rd->off);
}

void
manager_delete_blob(manager m, dirent d)
{
    blob b;
    region r;

    r = manager_get_region(m, d);
    b = manager_get_blob(m, d);
    region_delete_blob(r, b);
}

static int
region_close_to_full(region r)
{
    return 0;
}

static int
manager_read_regions(manager mgr, uint16_t count)
{
    int i, j, n = 0;

    mgr->all_regions = malloc(sizeof(struct region) * count);
    if (!mgr->all_regions) return warn("malloc"), -1;
    mgr->nregions = count;

    for (i = 0; i < mgr->nbundles; i++) {
        bundle bun = bundle_get(mgr, i);
        for (j = 0; j < bun->nregions; j++) {
            blob bl;
            size_t size;
            region reg = &mgr->all_regions[n++];

            // Total size of the region, including the headers
            size = 1 << REGION_BITS;
            if (j == bun->nregions - 1) size = bun->reg_size % size;
            // the last region might be shorter

            region_init(reg, n - 1, bundle_get_region_storage(bun, j), size);

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

int
manager_init(manager m)
{
    int i, r;
    size_t nregions = 0;

    m->directory = make_spht(0);
    if (!m->directory) return warnx("making directory"), -1;

    if (m->nbundles < 1) return raw_warnx("no bundles defined"), -1;

    for (i = 0; i < m->nbundles; i++) {
        bundle b = m->all_bundles + i;
        r = bundle_open(b);
        if (r == -1) {
            warnx("error with bundle %s; skipping", b->name);
            continue;
        }
        nregions += b->nregions;
    }

    if (nregions < 1) return warnx("no valid regions"), -2;

    r = manager_read_regions(m, nregions);
    if (r == -1) return warnx("manager_read_regions"), -2;

    return 0;
}

