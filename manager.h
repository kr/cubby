// manager.h -- structure containing global state

#ifndef manager_h
#define manager_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

typedef struct manager *manager;

#include "region.h"
#include "spht.h"

/* This is sort of a dumping ground for all the global state. This is slightly
   better than just having static global vars scattered around the various
   modules. */
struct manager {
    struct bundle *all_bundles;
    int nbundles;

    struct region *all_regions;
    int nregions;

    // Linked list of available regions. They are not necessarily empty.
    region free_regions_head;
    region free_regions_tail;

    spht directory;
};

int manager_init(manager m);

// Pick a region with enough capacity to hold size bytes.
region manager_pick_region(manager m, size_t size);

// Add this region to the free list.
void manager_add_free_region(manager m, region r);

blob manager_get_blob(manager m, dirent d);

#endif //manager_h
