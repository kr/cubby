// peer.c -- Network Peer Node

#include <stdlib.h>

#include "peer.h"
#include "util.h"

peer
make_peer(uint32_t *key)
{
    peer p;

    if (!key) return 0;

    p = malloc(sizeof(struct peer));
    if (!p) return warn("malloc"), (peer) 0;

    p->key[0] = key[0];
    p->key[1] = key[1];
    p->key[2] = key[2];

    return p;
}
