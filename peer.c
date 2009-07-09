// peer.c -- Network Peer Node

#include <stdlib.h>
#include <string.h>

#include "peer.h"
#include "cpkt.h"
#include "manager.h"
#include "util.h"

// How long (usec) to wait after the last message was received before sending a
// ping.
#define PING_INTERVAL (10 * SECOND)

// How long (usec) to wait after the last ping was sent before sending another
// ping.
#define REPING_INTERVAL (1 * SECOND)

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

static void
peer_send_ping(peer pr)
{
    cpkt pkt = make_cpkt_ping(pr->addr, pr->cp_port,
            pr->manager->memcache_port, pr->manager->http_port);
    if (!pkt) return warnx("make_cpkt_ping");

    pr->last_message_to = now_usec();

    manager_out_add(pr->manager, pkt);
}

void
peer_update(peer p)
{
    if (peer_needs_ping(p)) peer_send_ping(p);
}

