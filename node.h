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
    char pad[3];
    char is_local;

    peer peer;
};

node make_node(uint32_t *key, peer p);

int node_is_local(node n);
int node_is_remote(node n);
int node_is_active(node n);
void node_free(node n);
int nodes_are_congruent(node a, node b);

#endif //node_h
