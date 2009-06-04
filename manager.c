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
manager_allocate_blob(manager m, dirent d, size_t size)
{
    region r;
    blob b;

    r = manager_pick_region(m, size);
    if (!r) return 0;

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
