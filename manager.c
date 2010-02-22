#include <string.h>

#include "bundle.h"
#include "manager.h"
#include "prot.h"
#include "node.h"
#include "key.h"
#include "sha512.h"
#include "util.h"

void
manager_add_free_region(manager m, region r)
{
    heap_give(&m->region_pool, r);
}

/* This will REMOVE a region from the free pool. The caller is responsible for
   putting it back. */
region
manager_pick_region(manager m, size_t size)
{
    region r, tmp = 0;

    // Pull out regions until we find one that's got enough space.
    r = heap_take(&m->region_pool);
    while (r && !region_has_space_for_blob(r, size)) {
        r->next = tmp;
        tmp = r;
        r = heap_take(&m->region_pool);
    }

    // Put back the ones we didn't use.
    for (; tmp; tmp = tmp->next) heap_give(&m->region_pool, tmp);

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

    /* Put r back in the free pool. */
    heap_give(&m->region_pool, r);

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
            size_t size;
            region reg = &mgr->all_regions[n++];

            char *base = (char *) bundle_get_region_storage(bun, j);

            // Total size of the region, including the headers
            size = 1LU << REGION_BITS;
            // the last region might be shorter
            if (base + size > bundle_top(bun)) {
                size = bundle_top(bun) - base;
            }
            region_init(reg, n - 1, (region_storage) base, size);
            region_read(reg, mgr, bun->name);

            if (!region_close_to_full(reg)) {
                manager_add_free_region(mgr, reg);
            }
        }
        bundle_sync(bun, 1);
    }

    return 0;
}

static int
manager_node_onremove(arr a, void *item, size_t index)
{
    node_free((node) item);
    return 0; // ignored
}

/* This function uses privately allocated static memory. Caution is advised. */
static uint32_t *
manager_next_key(uint32_t *prev)
{
    static uint32_t next[3];

    sha512((char *) prev, 12, next, 12);
    return next;
}

void
manager_merge_nodes(manager m, uint16_t chain_len, uint32_t *key, peer p)
{
    if (!p) return warnx("no peer");
    for (int i = 0; i < chain_len; i++, key = manager_next_key(key)) {
        manager_merge_node(m, key, p);
    }
}

int
manager_init(manager m)
{
    int i, r;
    size_t nregions = 0;

    // Initialize the heap if necessary
    heap_init(&m->region_pool, (cmp_fn) region_space_cmp);

    arr_init(&m->nodes, 0, manager_node_onremove);
    arr_init(&m->outstanding_links, 0, prot_outstanding_link_onremove);

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
        m->key_chain_len += CEILDIV(b->reg_size, BUNDLE_OVERLAY_NODE_SIZE);
    }

    if (nregions < 1) return warnx("no valid regions"), -2;

    r = manager_read_regions(m, nregions);
    if (r == -1) return warnx("manager_read_regions"), -2;

    return 0;
}

static void
manager_grow_peers(manager m)
{
    size_t new_cap = (m->peers_cap + 1) << 1;
    peer *new = malloc(new_cap * sizeof(peer));

    if (!new) return warn("malloc");

    memcpy(new, m->peers, m->peers_fill * sizeof(peer));
    free(m->peers);
    m->peers = new;
    m->peers_cap = new_cap;
}

int
manager_insert_peer(manager m, peer p)
{

    if (m->peers_fill >= m->peers_cap) manager_grow_peers(m);
    if (m->peers_fill >= m->peers_cap) return -1;

    m->peers[m->peers_fill++] = p;
    return 0;
}

#include <stdio.h>

void
manager_out_add(manager m, cpkt c)
{
    c->next = 0;
    if (!m->out_head) {
        m->out_head = c;
    } else {
        m->out_tail->next = c;
    }
    m->out_tail = c;
}

int
manager_out_any(manager m)
{
    return !!m->out_head;
}

cpkt
manager_out_remove(manager m)
{
    cpkt c = m->out_head;

    if (!c) return 0;

    m->out_head = c->next;
    if (!m->out_head) m->out_tail = 0;

    return c;
}

void
manager_out_pushback(manager m, cpkt c)
{
    if (!m->out_head) m->out_tail = c;
    c->next = m->out_head;
    m->out_head = c;
}

/* If the peer does not exist, it will be created. */
peer
manager_get_peer(manager m, in_addr_t addr, uint16_t port)
{
    for (int i = 0; i < m->peers_fill; i++) {
        peer p = m->peers[i];
        if (p->addr == addr && p->cp_port == port) return p;
    }

    peer p = make_peer(m, addr, port);
    if (!p) return warnx("make_peer"), (peer) 0;

    int r = manager_insert_peer(m, p);
    if (r == -1) return warnx("manager_insert_peer"), (peer) 0;

    return p;
}

static void
manager_rebalance_dirent_cb(manager m, uint32_t *key, int error, void *ignore)
{
    if (error == 0) {
        // We might need to delete our dirent if we are no longer within
        // DIRENT_W neighbors of KEY. Same goes for our neighbors further away.
        // The usual linking algorithm covers this case, so we act like we just
        // got a LINK message with our new rank.

        dirent de = spht_get(m->directory, key);
        if (!de) return; // should not happen

        peer_id ids[de->len];
        for (int i = 0; i < de->len; i++) {
            rdesc rd = de->rdescs + i;
            if (rd->flags & RDESC_LOCAL) {
                ids[i] = peer_get_id(m->self);
            } else {
                ids[i] = make_peer_id(rd->b, rd->a);
            }
        }

        prot_link(m, key, de->len, ids, de->rank, 0, 0);
    } else {
        // Buh.
    }
}

static void
manager_rebalance_dirent(manager mgr, dirent de)
{
    // Continue only if we are the old owner.
    if (de->rank != 0) return;

    node owner;
    int n = manager_find_owners(mgr, de->key, 1, &owner);
    if (n < 1) return warnx("no active peers"); // can't happen

    // Continue only if the new owner is still bootstrapping.
    if (owner->peer->state != peer_state_in_rebalance) return;

    // Assume we are out of range. If this is wrong, our closer neighbor will
    // correct us.
    de->rank = DIRENT_W;

    prot_send_primary_link(mgr, de, manager_rebalance_dirent_cb, 0);
}

static void
manager_recover_dirent(manager mgr, dirent de)
{
    // Continue only if we aren't already the primary owner.
    if (de->rank == 1) return;

    node owner;
    int n = manager_find_owners(mgr, de->key, 1, &owner);
    if (n < 1) return warnx("no active peers"); // can't happen

    // Continue only if we are the new owner.
    if (owner->peer != mgr->self) return;

    // We will check for necessary file copies on the other side.
    prot_send_primary_link(mgr, de, manager_rebalance_dirent_cb, 0);
}

int
manager_find_owners(manager m, uint32_t *key, int n, node *out)
{
    node ps[n + 1];

    int found = 0;
    for (int i = 0; i < m->nodes.used; i++) {
        if (!node_is_active(m->nodes.items[i])) continue;

        int skip = 0;
        for (int j = 0; j < found; ++j) {
            if (nodes_are_congruent(ps[j], m->nodes.items[i])) skip = 1;
        }
        if (skip) continue;

        ps[found] = m->nodes.items[i];
        for (int j = found; j; j--) {
            if (key_distance_cmp(m->key, ps[j - 1]->key, ps[j]->key) < 0) break;
            node t = ps[j];
            ps[j] = ps[j - 1];
            ps[j - 1] = t;
        }
        found = min(found + 1, n);
    }

    for (int i = 0; i < found; i++) out[i] = ps[i];
    return found;
}

dirent
manager_add_links(manager m, uint32_t *key, uint8_t rank,
        int len, peer_id *peer_ids)
{
    return 0;

    int r, new_len = 0;
    peer_id new_ids_buf[len], *new_ids = 0;
    dirent nde, de = spht_get(m->directory, key);

    if (de) {
        new_ids = new_ids_buf;

        for (int i = 0; i < len; i++) {
            if (dirent_has_remote(de,
                        peer_id_get_addr(peer_ids[i]),
                        peer_id_get_port(peer_ids[i]))) continue;
            if (peer_ids[i] == peer_get_id(m->self)) continue;
            new_ids[new_len++] = peer_ids[i];
        }

        dirent_set_rank(de, rank);
        nde = copy_dirent(de, de->len + new_len);
    } else {
        nde = make_dirent(key, len, rank);
        new_len = len;
        new_ids = peer_ids;
    }

    if (!nde) return 0;

    for (int i = 0; i < new_len; i++) {
      r = dirent_add_remote(nde, peer_id_get_addr(new_ids[i]),
              peer_id_get_port(new_ids[i]));
      if (r == -1) return free(nde), (dirent) 0; // can't happen
    }

    r = spht_set(m->directory, nde);
    if (r == -1) return free(nde), (dirent) 0;

    free(de);

    return nde;
}

int
manager_merge_node(manager mgr, uint32_t *key, peer p)
{
    for (size_t i = 0; i < mgr->nodes.used; i++) {
        node n = mgr->nodes.items[i];
        if (n->key == key) {
            if (n->peer != p) warnx("node conflict %s -> %d:%d & %d:%d");
            n->peer = p;
            return 0;
        }
    }

    node n = make_node(key, p);
    if (!n) return warnx("make_node"), -1;
    int r = arr_append(&mgr->nodes, n);
    if (r != 1) return warnx("arr_append"), -1;
    return 0;
}

static int
manager_any_peers_need_work(manager mgr)
{
    for (int i = 0; i < mgr->peers_fill; i++) {
        if (mgr->peers[i]->state == peer_state_needs_rebalance ||
                mgr->peers[i]->state == peer_state_needs_recovery) return 1;
    }
    return 0;
}

void
manager_rebalance_work(manager mgr)
{
    int dirty = manager_any_peers_need_work(mgr);

    // Nothing to do? Just return.
    if (!dirty && !mgr->cursor.in_progress) return;

    // New dirty peers? Start over.
    if (dirty) {
        for (int i = 0; i < mgr->peers_fill; i++) {
            peer p = mgr->peers[i];
            if (p->state == peer_state_needs_rebalance) {
                p->state = peer_state_in_rebalance;
            }
            if (p->state == peer_state_needs_recovery) {
                p->state = peer_state_in_recovery;
            }
        }
        mgr->cursor.in_progress = 1;
        mgr->cursor.pos = 0;
        mgr->cursor.cap_check = mgr->directory->table->cap;
    }

    // Hash table grew? Start over.
    if (mgr->cursor.cap_check != mgr->directory->table->cap) {
        mgr->cursor.pos = 0;
        mgr->cursor.cap_check = mgr->directory->table->cap;
    }

    usec start = now_usec();
    size_t i;
    for (i = mgr->cursor.pos; i < mgr->directory->table->cap; i++) {
        // TODO: performance optimization: avoid so many syscalls; only check
        // the time every N iterations.
        if (now_usec() - start > 10000) break; // Stop after 10ms.

        dirent de = sparr_get(mgr->directory->table, i);
        if (!de || de == invalid_dirent) continue;

        manager_rebalance_dirent(mgr, de);
        manager_recover_dirent(mgr, de);
    }
    mgr->cursor.pos = i;

    // Got to the end?
    if (i == mgr->directory->table->cap) {
        mgr->cursor.in_progress = 0;
        for (int j = 0; j < mgr->peers_fill; j++) {
            peer p = mgr->peers[j];
            if (p->state == peer_state_in_rebalance) {
                p->state = peer_state_normal;
            }
        }
    }
}
