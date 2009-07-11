// cpkt.c -- Control protocol packet

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "cpkt.h"
#include "peer.h"
#include "util.h"

typedef struct cpkt_ping {
    CPKT_COMMON;

    // Bytes from the network
    uint8_t type;
    uint8_t pad[7];
    uint16_t memcache_port;
    uint16_t http_port;
} *cpkt_ping;

typedef struct cpkt_peer_desc {
    uint32_t addr;
    uint16_t port;
    uint8_t pad0; // reserved
    uint8_t pad1; // reserved
} *cpkt_peer_desc;

typedef struct cpkt_pong {
    CPKT_COMMON;

    // Bytes from the network
    uint8_t type;
    uint8_t pad[7];

    struct cpkt_peer_desc peers[];
} *cpkt_pong;

typedef struct cpkt_link {
    CPKT_COMMON;

    // Bytes from the network
    uint8_t type;
    uint8_t pad[7];

    uint32_t key[3];
} *cpkt_link;

typedef void(*cpkt_handle_fn)(cpkt, peer);

static void cpkt_ping_handle(cpkt cp, peer p);
static void cpkt_pong_handle(cpkt cp, peer p);
static void cpkt_link_handle(cpkt cp, peer p);

struct cpkt_type {
    const char *name;

    // The number of bytes in the smallest possible packet of this type.
    uint16_t base;

    // Size increment. Payload size (excluding the base) must be a multiple of
    // this. If the increment is zero, packet size must be equal to the base
    // size.
    uint16_t inc;

    cpkt_handle_fn fn;
};

#define CPKT_BASE_SIZE(t) \
    (sizeof(struct cpkt_##t) - sizeof(struct cpkt))

#define CPKT_TYPE(t, i) { \
    #t, CPKT_BASE_SIZE(t), i, cpkt_##t##_handle \
}

enum cpkt_type_codes {
    CPKT_TYPE_CODE_PING,
    CPKT_TYPE_CODE_PONG,
    CPKT_TYPE_CODE_LINK,
};

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
    return types[cpkt_get_type(p)].base;
}

inline int
cpkt_flex_size(cpkt p)
{
    return p->size - types[cpkt_get_type(p)].base;
}

int
cpkt_get_type(cpkt c) {
    if (!c) return -1;
    return c->data[0];
}

void
cpkt_set_type(cpkt c, uint8_t t)
{
    if (!c) return;
    c->data[0] = t;
}

uint8_t *
cpkt_body(cpkt c)
{
    if (!c) return 0;
    return c->data;
}

const char *
cpkt_type_name(cpkt c)
{
    return types[cpkt_get_type(c)].name;
}

inline cpkt
cpkt_check_size(cpkt p)
{
    uint16_t inc, trail;

    // Is it too small?
    if (p->size < cpkt_base_size(p)) return 0;

    trail = cpkt_flex_size(p);

    inc = types[cpkt_get_type(p)].inc;
    if (inc > 0) trail %= inc;

    // Is it an odd size?
    if (trail != 0) return 0;

    return p;
}

// Assumes correct type.
static void
cpkt_ping_handle(cpkt cp, peer p)
{
    cpkt_ping cp_ping = (cpkt_ping) cpkt_check_size(cp);

    if (!cp_ping) return warnx("cp %p is not a ping packet", cp);

    peer_send_pong(p);
}

static void
cpkt_pong_handle(cpkt cp, peer p)
{
    // FIXME: stub
    raw_warnx("got pong");
}

static void
cpkt_link_handle(cpkt cp, peer p)
{
    // FIXME: stub
    raw_warnx("got link");
}

void
cpkt_handle(cpkt cp, peer p)
{
    if (cpkt_get_type(cp) >= KNOWN_TYPES) {
        return warnx("ignoring message: unknown type %x", cpkt_get_type(cp));
    }

    types[cpkt_get_type(cp)].fn(cp, p);
}

cpkt
make_cpkt(uint16_t size)
{
    cpkt cp;

    cp = malloc(sizeof(struct cpkt) + size);
    if (!cp) return warn("malloc"), (cpkt) 0;

    memset(cp, 0, sizeof(struct cpkt) + size);

    // The number of bytes in the packet, not including the common header.
    // There will be size - 1 bytes in data (the first byte is the type
    // member).
    cp->size = size;

    return cp;
}

cpkt
make_cpkt_ping(in_addr_t addr, int cp_port, int memcache_port, int http_port)
{
    cpkt_ping cp = (cpkt_ping) make_cpkt(CPKT_BASE_SIZE(ping));
    if (!cp) return warnx("make_cpkt"), (cpkt) 0;

    cpkt_set_type((cpkt) cp, CPKT_TYPE_CODE_PING);

    cp->memcache_port = memcache_port;
    cp->http_port = http_port;
    return (cpkt) cp;
}

cpkt
make_cpkt_pong(in_addr_t addr, uint16_t port, peer *peers, int len)
{
    cpkt_pong cp = (cpkt_pong) make_cpkt(CPKT_BASE_SIZE(pong) +
            sizeof(struct cpkt_peer_desc) * len);
    if (!cp) return warnx("make_cpkt"), (cpkt) 0;

    cpkt_set_type((cpkt) cp, CPKT_TYPE_CODE_PONG);

    for (int i = 0; i < len; i++) {
        cp->peers[i].addr = peers[i]->addr;
        cp->peers[i].port = peers[i]->cp_port;
    }
    return (cpkt) cp;
}

cpkt
make_cpkt_link(uint32_t *key)
{
    cpkt_link c = (cpkt_link) make_cpkt(CPKT_BASE_SIZE(link));
    if (!c) return warnx("make_cpkt"), (cpkt) 0;

    cpkt_set_type((cpkt) c, CPKT_TYPE_CODE_LINK);
    c->key[0] = key[0];
    c->key[1] = key[1];
    c->key[2] = key[2];
    return (cpkt) c;
}

void
cpkt_error(cpkt c, int err)
{
    errno = err;
    warn("cpkt error");
}
