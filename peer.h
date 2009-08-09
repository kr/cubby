// peer.h -- Network Peer Node header

#ifndef peer_h
#define peer_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

#include <netinet/in.h>

typedef uint64_t peer_id;

typedef struct peer *peer;

typedef enum peer_states {
    peer_state_invalid,
    peer_state_normal,
    peer_state_needs_rebalance,
    peer_state_in_rebalance,
    peer_state_needs_recovery,
    peer_state_in_recovery,
} peer_state;

#include "manager.h"

struct peer {
    manager manager;
    uint32_t key[3];
    in_addr_t addr;
    int cp_port; // Cubby UDP control protocol port
    peer_state state;

    uint64_t last_message_to; // Local time stamp of our last sent packet.
    uint64_t last_message_from; // Local time stamp of the last received packet.
};

peer make_peer(manager mgr, in_addr_t addr, int cp_port);
peer_id make_peer_id(in_addr_t addr, uint16_t cp_port);

peer_id peer_get_id(peer p);
in_addr_t peer_id_get_addr(peer_id id);
uint16_t peer_id_get_port(peer_id id);

void peer_update(peer p);
void peer_touch(peer p);
void peer_send_pong(peer p);
void peer_send_link(peer p, dirent de, uint8_t rank);
void peer_send_linked(peer p, uint32_t *key);
int peer_active(peer p);

#endif //peer_h
