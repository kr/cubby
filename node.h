// node.h -- Network Peer Node header

#ifndef node_h
#define node_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

typedef struct node *node;

#include "bundle.h"
#include "peer.h"

struct node {
    uint32_t key[3];
    char pad[4];

    // Exactly one of these should be non-null.
    bundle bundle;
    peer peer;
};

node make_node_local(uint32_t *key, bundle b);
node make_node_remote(uint32_t *key, peer p);

int node_is_local(node n);
int node_is_active(node n);
void node_free(node n);

#endif //node_h
