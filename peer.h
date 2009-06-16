// peer.h -- Network Peer Node header

#ifndef peer_h
#define peer_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

#include <netinet/in.h>

typedef struct peer *peer;

//#include "net.h"

struct peer {
    uint32_t key[3];
    struct in_addr addr;
    int cp_port; // Cubby UDP control protocol port
};

peer make_peer(uint32_t *key);

#endif //peer_h
