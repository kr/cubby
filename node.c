// node.c -- Network Peer Node

#include "node.h"

node
make_node(uint32_t *key, peer p)
{
    if (!key) return warnx("no key"), (node) 0;
    if (!p) return warnx("no peer"), (node) 0;

    node n = malloc(sizeof(struct node));
    if (!n) return warn("malloc"), (node) 0;

    n->key[0] = key[0];
    n->key[1] = key[1];
    n->key[2] = key[2];
    n->peer = p;

    return n;
}

int
node_is_local(node n)
{
    return !!n->peer->is_local;
}

int
node_is_remote(node n)
{
    return !node_is_local(n);
}

int
node_is_active(node n)
{
    return node_is_local(n) || peer_active(n->peer);
}

void
node_free(node n)
{
    free(n);
}

/* Return 1 if the nodes both refer to the local process, or both refer to the
   same peer, and 0 otherwise. */
int
nodes_are_congruent(node a, node b)
{
    return a->peer == b->peer;
}
