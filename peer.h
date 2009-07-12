// peer.h -- Network Peer Node header

#ifndef peer_h
#define peer_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

#include <netinet/in.h>

typedef struct peer *peer;

#include "manager.h"

struct peer {
    manager manager;
    uint32_t key[3];
    in_addr_t addr;
    int cp_port; // Cubby UDP control protocol port

    uint64_t last_message_to; // Local time stamp of our last sent packet.
    uint64_t last_message_from; // Local time stamp of the last received packet.
};

peer make_peer(manager mgr, in_addr_t addr, int cp_port);

void peer_update(peer p);
void peer_touch(peer p);
void peer_send_pong(peer p);
void peer_send_link(peer p, uint32_t *key);
void peer_send_linked(peer p, uint32_t *key);
int peer_active(peer p);

#endif //peer_h
