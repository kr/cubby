// peer.c -- Network Peer Node

#include <stdlib.h>
#include <string.h>

#include "peer.h"
#include "cpkt.h"
#include "manager.h"
#include "util.h"

// How long (usec) to wait after the last message was received before
// considering a peer to be inactive.
#define ACTIVE_INTERVAL (20 * SECOND)

// How long (usec) to wait after the last message was received before sending a
// ping.
#define PING_INTERVAL (10 * SECOND)

// How long (usec) to wait after the last ping was sent before sending another
// ping.
#define REPING_INTERVAL (1 * SECOND)

#define PONG_COUNT 45

peer
make_peer(manager mgr, in_addr_t addr, int cp_port)
{
    peer p;

    p = malloc(sizeof(struct peer));
    if (!p) return warn("malloc"), (peer) 0;

    memset(p, 0, sizeof(struct peer));
    p->manager = mgr;
    p->addr = addr;
    p->cp_port = cp_port;

    // Compute key.
    key_for_peer(p->key, addr, cp_port);

    return p;
}

static int
peer_needs_ping(peer p)
{
    int64_t now = now_usec();
    int64_t delta_to = now - p->last_message_to;
    int64_t delta_from = now - p->last_message_from;

    return delta_from > PING_INTERVAL && delta_to > REPING_INTERVAL;
}

int
peer_active(peer p)
{
    int64_t now = now_usec();
    int64_t delta_from = now - p->last_message_from;

    return delta_from < ACTIVE_INTERVAL;
}

static void
peer_send(peer p, cpkt c)
{
    p->last_message_to = now_usec();
    c->remote_addr = p->addr;
    c->remote_port = p->cp_port;
    manager_out_add(p->manager, c);
}

void
peer_touch(peer p)
{
    if (!p) return;
    p->last_message_from = now_usec();
}

static void
peer_send_ping(peer pr)
{
    cpkt pkt = make_cpkt_ping(pr->addr, pr->cp_port,
            pr->manager->memcache_port, pr->manager->http_port);
    if (!pkt) return warnx("make_cpkt_ping");

    peer_send(pr, pkt);
}

void
peer_send_pong(peer p)
{
    manager m = p->manager;
    peer closest[PONG_COUNT];

    if (!p) return;

    int n = manager_find_closest_active_peers(m, m->key, PONG_COUNT, closest);
    cpkt pkt = make_cpkt_pong(0, 0, closest, n);
    if (!pkt) return warnx("make_cpkt_pong");

    peer_send(p, pkt);
}

void
peer_send_link(peer p, uint32_t *key)
{
    cpkt c = make_cpkt_link(key);
    if (!c) return warnx("make_cpkt_link");

    peer_send(p, c);
}

void
peer_send_linked(peer p, uint32_t *key)
{
    cpkt c = make_cpkt_linked(key);
    if (!c) return warnx("make_cpkt_linked");

    peer_send(p, c);
}

void
peer_update(peer p)
{
    if (peer_needs_ping(p)) peer_send_ping(p);
}
