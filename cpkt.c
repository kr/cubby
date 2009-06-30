// cpkt.c -- Control protocol packet

#include <stddef.h>

#include "cpkt.h"
#include "util.h"

typedef struct cpkt_ping {
    uint32_t src_addr;
    uint16_t src_port;
    uint16_t size;
    void *end;

    // Bytes from the network
    uint8_t type;
    uint8_t pad[7];
    uint16_t memcache_port;
    uint16_t http_port;
} *cpkt_ping;

typedef struct cpkt_pong {
    uint32_t src_addr;
    uint16_t src_port;
    uint16_t size;
    void *end;

    // Bytes from the network
    uint8_t type;
    uint8_t pad[7];
} *cpkt_pong;

typedef struct cpkt_link {
    uint32_t src_addr;
    uint16_t src_port;
    uint16_t size;
    void *end;

    // Bytes from the network
    uint8_t type;
    uint8_t pad[7];
} *cpkt_link;

typedef void(*cpkt_handle_fn)(cpkt);

struct cpkt_type {
    // The number of bytes in the smallest possible packet of this type.
    uint16_t base;

    // Size increment. Payload size (excluding the base) must be a multiple of
    // this. If the increment is zero, packet size must be equal to the base
    // size.
    uint16_t inc;

    cpkt_handle_fn fn;
};

#define CPKT_BASE_SIZE(t) \
    (sizeof(struct cpkt_##t) - offsetof(struct cpkt, type))

#define CPKT_TYPE(t, i) { \
    CPKT_BASE_SIZE(t), i, cpkt_##t##_handle \
}

#define KNOWN_TYPES 3
static struct cpkt_type types[KNOWN_TYPES] = {
    CPKT_TYPE(ping, 0),
    CPKT_TYPE(pong, 0),
    CPKT_TYPE(link, 0),
};

static inline int
is_mul(int inc, int size)
{
    return inc == 0 ? size == 0 : size % inc == 0;
}

inline int
cpkt_base_size(cpkt p)
{
    return types[p->type].base;
}

inline int
cpkt_flex_size(cpkt p)
{
    return p->size - types[p->type].base;
}

inline cpkt
cpkt_check_size(cpkt p)
{
    uint16_t inc, trail;

    // Is it too small?
    if (p->size < cpkt_base_size(p)) return 0;

    trail = cpkt_flex_size(p);

    inc = types[p->type].inc;
    if (inc > 0) trail %= inc;

    // Is it an odd size?
    if (trail != 0) return 0;

    return p;
}

// Assumes correct cp->type.
void
cpkt_ping_handle(cpkt cp)
{
    cpkt_ping p = (cpkt_ping) cpkt_check_size(cp);

    if (!p) return warnx("cp %p is not a ping packet", cp);

    // FIXME: stub
    raw_warnx("got ping");
}

void
cpkt_pong_handle(cpkt cp)
{
    // FIXME: stub
    raw_warnx("got pong");
}

void
cpkt_link_handle(cpkt cp)
{
    // FIXME: stub
    raw_warnx("got link");
}

void
cpkt_handle(cpkt cp)
{
    if (cp->type >= KNOWN_TYPES) {
        return warnx("ignoring message: unknown type %d", cp->type);
    }

    types[cp->type].fn(cp);
}

cpkt
make_cpkt(uint16_t size)
{
    cpkt cp;

    cp = malloc(offsetof(struct cpkt, type) + size);
    if (!cp) return warn("malloc"), (cpkt) 0;

    cp->size = size;

    return cp;
}

