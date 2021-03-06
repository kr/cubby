// prot.c -- Protocol

#include "prot.h"
#include "key.h"
#include "manager.h"

#define ABANDON_LINK_INTERVAL (200 * MSEC)
#define RETRY_LINK_INTERVAL (50 * MSEC)

typedef struct link_progress {
    manager manager;
    uint32_t key[3];
    uint8_t rank;

    prot_send_link_fn cb;
    void *data;

    peer peer;
    usec first_at;
    usec last_at;
} *link_progress;

int
prot_outstanding_link_onremove(arr a, void *item, size_t index)
{
    free(item);
    return 0; // ignored
}

static int
prot_outstanding_link_update(arr a, void *item, size_t index)
{
    link_progress prog = item;

    if (!prog->peer) {
        // It's done! Let the callback know.
        prog->cb(prog->manager, prog->key, error_code_success, prog->data);
        return 0; // remove it from the list
    }

    usec now = prog->peer->manager->slice_start;
    usec delta_last = now - prog->last_at;
    usec delta_first = now - prog->first_at;

    dirent de = spht_get(prog->manager->directory, prog->key);
    if (!de) { // Wtf? it's gone. Nothing left to do now.
        prog->cb(prog->manager, prog->key, error_code_missing_dirent, prog->data);
        return 0; // remove it from the list
    }

    if (prog->first_at == 0) { // Send the first request
        prog->first_at = now;
        prog->last_at = now;
        peer_send_link(prog->peer, de, prog->rank);
    } else if (delta_first > ABANDON_LINK_INTERVAL) {
        prog->cb(prog->manager, de->key, error_code_no_reply, prog->data);
        return 0; // remove it from the list
    } else if (delta_last > RETRY_LINK_INTERVAL) {
        prog->last_at = now;
        peer_send_link(prog->peer, de, prog->rank);
    }
    // else do nothing

    return 1; // keep it in the list
}

void
prot_linked(peer p, uint32_t *key)
{
    arr a = &p->manager->outstanding_links;

    for (int i = 0; i < a->used; i++) {
        link_progress prog = a->items[i];
        if (!key_eq(key, prog->key)) continue;
        if (prog->peer == p) {
            prog->peer = 0;
        }
    }
}

void
prot_work(manager mgr)
{
    mgr->slice_start = now_usec();
    for (int i = 0; i < mgr->peers_fill; i++) {
        peer_update(mgr->peers[i]);
    }
    manager_rebalance_work(mgr);

    arr_filter(&mgr->outstanding_links, prot_outstanding_link_update);
}


void
prot_init()
{
}

static void
prot_send_link(manager m, peer *to, dirent de, uint8_t rank,
        prot_send_link_fn cb, void *data)
{
    link_progress prog = malloc(sizeof(struct link_progress));
    if (!prog) return cb(m, de->key, error_code_no_mem, data);

    memset(prog, 0, sizeof(struct link_progress));

    prog->manager = m;
    prog->key[0] = de->key[0];
    prog->key[1] = de->key[1];
    prog->key[2] = de->key[2];
    prog->rank = rank;
    prog->cb = cb;
    prog->data = data;

    prog->peer = to[0];

    arr_append(&m->outstanding_links, prog);
}

void
prot_send_primary_link(manager m, dirent de, prot_send_link_fn cb, void *data)
{
    node closest;
    int n = manager_find_owners(m, de->key, 1, &closest,
            manager_find_owners_none);

    // can't happen -- should at least find ourselves
    if (!n) return cb(m, de->key, error_code_insufficient_nodes, data);

    prot_send_link(m, &closest->peer, de, 0, cb, data);
}

void
prot_start_copies(manager m, dirent de)
{
    // Continue only if we are the primary owner.
    if (de->rank != 0) return;

    // TODO fill out this stub

    // TODO A node that becomes inactive briefly, then becomes active again,
    // might still have some or all its files. We should check if we have any
    // stale pointers to such a node, and, if so, prefer to send the file to
    // that node. This lets us avoid recopying the file if the node still has
    // it.
}

/* This is the meat of the distributed linking algorithm. */
void
prot_link(manager m, uint32_t *key, int len, peer_id *peer_ids, uint8_t rank,
        prot_link_fn cb, void *data)
{
    dirent de;
    if (rank < DIRENT_W) {
        node nodes[rank + 2];
        int n = manager_find_owners(m, key, rank + 2, nodes,
                manager_find_owners_none);

        // Oops, they claim we have rank rank, but we don't even know that many
        // distinct nodes. We must be missing some nodes.
        if (rank >= n) {
          // TODO something useful
          return cb(m, key, error_code_insufficient_nodes, data);
        }

        // We seem to disagree about our proper rank.
        if (!node_is_local(nodes[rank])) {
          // TODO something useful
          return cb(m, key, error_code_rank_mismatch, data);
        }

        node next = nodes[rank + 1];

        // store entry for T under key K at rank R
        de = manager_add_links(m, key, rank, len, peer_ids);
        if (!de) return cb(m, key, error_code_add_links, data);

        // No more nodes? We are last in the known order of succession.
        if (rank + 1 >= n) return cb(m, key, error_code_success, data);

        // LINK(K, T, R + 1) -> C
        // when LINKED(K) <- C
        //   LINKED(K) -> A
        // Pass our continuation directly to this tail call.
        prot_send_link(m, &next->peer, de, rank + 1, cb, data);

        // Are we the primary owner? We are in charge of replication.
        if (rank == 0) prot_start_copies(m, de);
    } else if ((de = spht_get(m->directory, key))) {
        // delete entry under key K
        // C = next closest node
        // LINK(K, T, R + 1) -> C
        // when LINKED(K) <- C
        // LINKED(K) -> A
    } else {
        return cb(m, key, error_code_success, data);
    }
}
