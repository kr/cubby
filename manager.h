// manager.h -- structure containing global state

#ifndef manager_h
#define manager_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

typedef struct manager *manager;

#include "region.h"
#include "bundle.h"
#include "arr.h"
#include "heap.h"
#include "spht.h"
#include "peer.h"
#include "node.h"
#include "cpkt.h"
#include "util.h"

/* This is sort of a dumping ground for all the global state. This is slightly
   better than just having static global vars scattered around the various
   modules. */
struct manager {
    uint32_t key[3]; // Our own id
    uint32_t key_chain_len;

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
    peer self;

    int memcache_port;
    int http_port;

    struct arr nodes;

    cpkt out_head, out_tail;

    struct arr outstanding_links;

    struct {
        size_t pos;
        size_t cap_check;
        int in_progress;
    } cursor;

    usec slice_start;
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

peer manager_get_peer(manager m, in_addr_t addr, uint16_t port);

/* DEPRECATED */
int manager_find_closest_active_peers(manager m, uint32_t *key, int n,
        peer *out);

/* DEPRECATED */
int manager_find_closest_active_remote_nodes(manager m, uint32_t *key, int n,
        node *out);

/* Fills out with at most n nodes in order of distance from key. Every node in
   out refers to a distinct peer. Returns the number of nodes placed in out.
   This may be less than n if fewer than n suitable nodes exist. */
int manager_find_owners(manager m, uint32_t *key, int n, node *out);

dirent manager_add_links(manager m, uint32_t *key, uint8_t rank,
        int len, peer_id *peer_ids);

int manager_merge_node(manager m, uint32_t *key, peer p);

void manager_merge_nodes(manager m, uint16_t chain_len, uint32_t *root_key,
        peer p);

void manager_rebalance_work(manager mgr);

#endif //manager_h
