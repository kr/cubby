// prot.c -- Protocol

#include "prot.h"
#include "key.h"
#include "manager.h"

#define DIRENT_W 2
#define ABANDON_LINK_INTERVAL (200 * MSEC)
#define RETRY_LINK_INTERVAL (50 * MSEC)

typedef struct link_progress {
    manager manager;
    uint32_t key[3];

    prot_send_link_fn cb;
    void *data;

    struct {
        peer peer;
        uint64_t first_at;
        uint64_t last_at;
    } peers[2];
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
        prog->cb(prog->manager, prog->key, 0, prog->data);
        return 0; // remove it from the list
    }

    for (int i = 0; i < DIRENT_W; i++) {
        peer p = prog->peers[i].peer;
        if (p) {
            int64_t now = now_usec();
            int64_t delta_last = now - prog->peers[i].last_at;
            int64_t delta_first = now - prog->peers[i].first_at;

            if (prog->peers[i].first_at == 0) { // Send the first request
                prog->peers[i].first_at = now;
                prog->peers[i].last_at = now;
                peer_send_link(p, prog->key);
            } else if (delta_first > ABANDON_LINK_INTERVAL) {
                prog->cb(prog->manager, prog->key, 1, prog->data);
                return 0; // remove it from the list
            } else if (delta_last > RETRY_LINK_INTERVAL) {
                prog->peers[i].last_at = now;
                peer_send_link(p, prog->key);
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
        if (!key_eq(key, prog->key)) continue;
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
    for (int i = 0; i < mgr->peers_fill; i++) {
        peer_update(mgr->peers[i]);
    }

    arr_filter(&mgr->outstanding_links, prot_outstanding_link_update);
}


void
prot_init()
{
}

void
prot_send_links(manager m, uint32_t *k, prot_send_link_fn cb, void *data)
{
    link_progress prog = malloc(sizeof(struct link_progress));
    if (!prog) return cb(m, k, 1, data);

    memset(prog, 0, sizeof(struct link_progress));

    prog->manager = m;
    prog->key[0] = k[0];
    prog->key[1] = k[1];
    prog->key[2] = k[2];
    prog->cb = cb;
    prog->data = data;

    peer closest[DIRENT_W];
    int n = manager_find_closest_active_peers(m, k, DIRENT_W, closest);
    for (int i = 0; i < n; i++) {
        prog->peers[i].peer = closest[i];
    }

    arr_append(&m->outstanding_links, prog);
}
