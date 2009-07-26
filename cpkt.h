// cpkt.h -- Control protocol packet header

#ifndef cpkt_h
#define cpkt_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

#include <netinet/in.h>

typedef struct cpkt *cpkt;

#include "manager.h"
#include "peer.h"

struct cpkt {
#define CPKT_COMMON \
    cpkt next; \
    in_addr_t remote_addr; \
    uint16_t remote_port; \
    uint16_t size; \
    void *end

    CPKT_COMMON;

    // Bytes from the network
    uint8_t data[];
};

cpkt make_cpkt(uint16_t size);
cpkt make_cpkt_ping(in_addr_t addr, int cp_port, manager mgr);
cpkt make_cpkt_pong(in_addr_t addr, uint16_t port, peer *peers, int len);
cpkt make_cpkt_link(uint32_t *key);
cpkt make_cpkt_linked(uint32_t *key);

cpkt cpkt_check_size(cpkt p);
int cpkt_base_size(cpkt p);
int cpkt_flex_size(cpkt p);
int cpkt_get_type(cpkt c);
const char *cpkt_type_name(cpkt c);
void cpkt_set_type(cpkt c, uint8_t t);
uint8_t * cpkt_body(cpkt c);

void cpkt_handle(cpkt cp, peer p);
void cpkt_error(cpkt c, int err);

#endif //cpkt_h
