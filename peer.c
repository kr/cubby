// peer.c -- Network Peer Node

#include "peer.h"

peer
make_peer(uint32_t *key)
{
    peer p;

    if (!key) return 0;

    p = malloc(sizeof(struct peer));
    if (!p) return warn("malloc"), 0;

    p->key[0] = key[0];
    p->key[1] = key[1];
    p->key[2] = key[2];

    return p;
}
