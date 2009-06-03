#include <string.h>

#include "bundle.h"
#include "manager.h"
#include "util.h"

int
manager_init(manager m)
{
    int r;

    m->directory = make_spht(0);
    if (!m->directory) return warnx("making directory"), -1;

    r = bundles_init(m); // Read and index the files
    if (r < 0) return r;

    return 0;
}

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
    return m->free_regions_head;
}

blob
manager_get_blob(manager m, dirent d)
{
    rdesc_local rd;
    region r;
    int i;

    for (i = 0; i < d->len; i++) {
        if (d->rdescs[i].flags & RDESC_LOCAL) break;
    }
    if (i >= d->len) return 0;

    rd = (rdesc_local) &d->rdescs[i];
    r = &m->all_regions[rd->reg];
    return region_get_blob_by_off(r, rd->off);
}

