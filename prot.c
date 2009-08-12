// prot.c -- Protocol

#include "prot.h"
#include "key.h"
#include "manager.h"

#define ABANDON_LINK_INTERVAL (200 * MSEC)
#define RETRY_LINK_INTERVAL (50 * MSEC)

typedef struct link_progress {
    manager manager;
    dirent de;
    uint8_t rank;

    prot_send_link_fn cb;
    void *data;

    struct {
        peer peer;
        usec first_at;
        usec last_at;
    } peers[DIRENT_W];
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

    if (!prog->peers[0].peer && !prog->peers[0].peer) {
        prog->cb(prog->manager, prog->de->key, 0, prog->data);
        return 0; // remove it from the list
    }

    for (int i = 0; i < DIRENT_W; i++) {
        peer p = prog->peers[i].peer;
        if (p) {
            usec now = p->manager->slice_start;
            usec delta_last = now - prog->peers[i].last_at;
            usec delta_first = now - prog->peers[i].first_at;

            if (prog->peers[i].first_at == 0) { // Send the first request
                prog->peers[i].first_at = now;
                prog->peers[i].last_at = now;
                peer_send_link(p, prog->de, prog->rank);
            } else if (delta_first > ABANDON_LINK_INTERVAL) {
                prog->cb(prog->manager, prog->de->key, 1, prog->data);
                return 0; // remove it from the list
            } else if (delta_last > RETRY_LINK_INTERVAL) {
                prog->peers[i].last_at = now;
                peer_send_link(p, prog->de, prog->rank);
            }
            // else do nothing
        }
    }

    return 1; // keep it in the list
}

void
prot_linked(peer p, uint32_t *key)
{
    arr a = &p->manager->outstanding_links;

    for (int i = 0; i < a->used; i++) {
        link_progress prog = a->items[i];
        if (!key_eq(key, prog->de->key)) continue;
        for (int j = 0; j < DIRENT_W; j++) {
            if (prog->peers[j].peer == p) {
                prog->peers[j].peer = 0;
            }
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

void
prot_send_links(manager m, int n, peer *to, dirent de, uint8_t rank,
        prot_send_link_fn cb, void *data)
{
    link_progress prog = malloc(sizeof(struct link_progress));
    if (!prog) return cb(m, de->key, 1, data);

    memset(prog, 0, sizeof(struct link_progress));

    prog->manager = m;
    prog->de = de;
    prog->rank = rank;
    prog->cb = cb;
    prog->data = data;

    if (n > DIRENT_W) {
        warnx("n > DIRENT_W (%d > %d) -- capping", n, DIRENT_W);
        n = DIRENT_W;
    }
    for (int i = 0; i < n; i++) {
        prog->peers[i].peer = to[i];
    }

    arr_append(&m->outstanding_links, prog);
}

void
prot_send_primary_link(manager m, dirent de, prot_send_link_fn cb, void *data)
{
    node closest;
    int n = manager_find_owners(m, de->key, 1, &closest);

    // can't happen -- should at least find ourselves
    if (!n) return cb(m, de->key, 0, data);

    prot_send_links(m, n, &closest->peer, de, 0, cb, data);
}

/* This is the meat of the distributed linking algorithm. */
void
prot_link(manager m, uint32_t *key, int len, peer_id *peer_ids, uint8_t rank,
        prot_link_fn cb, void *data)
{
    dirent de;
    if (rank < DIRENT_W) {
        node nodes[rank + 2];
        int n = manager_find_owners(m, key, rank + 2, nodes);

        // Oops, they claim we have rank rank, but we don't even know that many
        // distinct nodes. We must be missing some nodes.
        if (n < rank + 1) {
          // TODO something useful
          return cb(m, key, 3, data);
        }

        // We seem to disagree about our proper rank.
        if (!node_is_local(nodes[rank])) {
          // TODO something useful
          return cb(m, key, 2, data);
        }

        node next = nodes[rank + 1];

        // store entry for T under key K at rank R
        de = manager_add_links(m, key, rank, len, peer_ids);
        if (!de) return cb(m, key, 1, data);

        // No more nodes? We are last in the known order of succession.
        if (n < rank + 2) return cb(m, key, 0, data);

        // LINK(K, T, R + 1) -> C
        // when LINKED(K) <- C
        //   LINKED(K) -> A
        // Pass our continuation directly to this tail call.
        prot_send_links(m, 1, &next->peer, de, rank + 1, cb, data);
    } else if ((de = spht_get(m->directory, key))) {
        // delete entry under key K
        // C = next closest node
        // LINK(K, T, R + 1) -> C
        // when LINKED(K) <- C
        // LINKED(K) -> A
    } else {
        return cb(m, key, 0, data);
    }
}
