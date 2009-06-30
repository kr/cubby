// cpkt.h -- Control protocol packet header

#ifndef cpkt_h
#define cpkt_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

typedef struct cpkt *cpkt;

struct cpkt {
    uint32_t src_addr;
    uint16_t src_port;
    uint16_t size;
    void *end;

    // Bytes from the network
    uint8_t type;
    uint8_t data[];
};

cpkt make_cpkt(uint16_t size);
cpkt cpkt_check_size(cpkt p);
int cpkt_base_size(cpkt p);
int cpkt_flex_size(cpkt p);

void cpkt_ping_handle(cpkt cp);
void cpkt_pong_handle(cpkt cp);
void cpkt_link_handle(cpkt cp);

void cpkt_handle(cpkt cp);

#endif //cpkt_h
