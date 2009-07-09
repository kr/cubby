// manager.h -- structure containing global state

#ifndef manager_h
#define manager_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

typedef struct manager *manager;

#include "region.h"
#include "heap.h"
#include "spht.h"
#include "peer.h"
#include "cpkt.h"

/* This is sort of a dumping ground for all the global state. This is slightly
   better than just having static global vars scattered around the various
   modules. */
struct manager {
    uint32_t key[3]; // Our own id

    struct bundle *all_bundles;
    int nbundles;

    struct region *all_regions;
    int nregions;

    // Regions with available space. They are not necessarily empty.
    struct heap region_pool;

    spht directory;

    peer *peers; // All known peers
    size_t peers_cap;
    size_t peers_fill;

    int memcache_port;
    int http_port;

    cpkt out_head, out_tail;
};

int manager_init(manager m);

// Pick a region with enough capacity to hold size bytes.
region manager_pick_region(manager m, size_t size);

// Add this region to the free list.
void manager_add_free_region(manager m, region r);

// Allocate a new blob and record its location in the dirent.
blob manager_allocate_blob(manager m, dirent d, size_t size);

void manager_delete_blob(manager m, dirent d);

region manager_get_region(manager m, dirent d);
blob manager_get_blob(manager m, dirent d);

int manager_insert_peer(manager m, peer p);

void manager_out_add(manager m, cpkt c);
int manager_out_any(manager m);
cpkt manager_out_remove(manager m);
void manager_out_pushback(manager m, cpkt c);

#endif //manager_h
