// node.c -- Network Peer Node

#include "node.h"

static node
make_node(uint32_t *key, bundle b, peer p)
{
    if (!key) return 0;

    node n = malloc(sizeof(struct node));
    if (!n) return warn("malloc"), (node) 0;

    n->key[0] = key[0];
    n->key[1] = key[1];
    n->key[2] = key[2];
    n->bundle = b;
    n->peer = p;

    return n;
}

node
make_node_local(uint32_t *key, bundle b)
{
    if (!b) return 0;
    return make_node(key, b, 0);
}

node
make_node_remote(uint32_t *key, peer p)
{
    if (!p) return 0;
    return make_node(key, 0, p);
}

int
node_is_local(node n)
{
    return !!n->bundle;
}

int
node_is_remote(node n)
{
    return !!n->peer;
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
