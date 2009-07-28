// cpkt.c -- Control protocol packet

#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "cpkt.h"
#include "prot.h"
#include "peer.h"
#include "sha512.h"
#include "util.h"

typedef struct cpkt_root_key_desc {
    uint32_t key[3];
    uint16_t chain_len;
    uint16_t pad; // reserved
} *cpkt_root_key_desc;

typedef struct cpkt_ping {
    CPKT_COMMON;

    // Bytes from the network
    uint8_t type;
    uint8_t pad[9];
    uint16_t memcache_port;
    uint16_t http_port;
    uint16_t root_key_count;

    struct cpkt_root_key_desc root_keys[];
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

    struct cpkt_peer_desc peers[PONG_PEER_MAX];
    struct cpkt_root_key_desc root_keys[];
} *cpkt_pong;

typedef struct cpkt_link {
    CPKT_COMMON;

    // Bytes from the network
    uint8_t type;
    uint8_t pad[11];

    uint32_t key[3];
} *cpkt_link;

typedef struct cpkt_linked {
    CPKT_COMMON;

    // Bytes from the network
    uint8_t type;
    uint8_t pad[11];

    uint32_t key[3];
} *cpkt_linked;

typedef void(*cpkt_handle_fn)(cpkt, peer);

static void cpkt_ping_handle(cpkt cp, peer p);
static void cpkt_pong_handle(cpkt cp, peer p);
static void cpkt_link_handle(cpkt cp, peer p);
static void cpkt_linked_handle(cpkt cp, peer p);

typedef struct cpkt_type {
    const char *name;

    // The number of bytes in the smallest possible packet of this type.
    uint16_t base;

    // Size increment. Payload size (excluding the base) must be a multiple of
    // this. If the increment is zero, packet size must be equal to the base
    // size.
    uint16_t inc;

    cpkt_handle_fn fn;
} *cpkt_type;

#define CPKT_BASE_SIZE(t) \
    (sizeof(struct cpkt_##t) - sizeof(struct cpkt))

#define CPKT_TYPE(t, i) { \
    #t, CPKT_BASE_SIZE(t), i, cpkt_##t##_handle \
}

enum cpkt_type_codes {
    CPKT_TYPE_CODE_PING,
    CPKT_TYPE_CODE_PONG,
    CPKT_TYPE_CODE_LINK,
    CPKT_TYPE_CODE_LINKED,
    KNOWN_CPKT_TYPE_CODES,
};

static struct cpkt_type types[KNOWN_CPKT_TYPE_CODES] = {
    CPKT_TYPE(ping, 0),
    CPKT_TYPE(pong, 0),
    CPKT_TYPE(link, 0),
    CPKT_TYPE(linked, 0),
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
cpkt_check_size(cpkt p, int *len)
{
    uint16_t inc, trail, my_len = 0;

    // Is it too small?
    if (p->size < cpkt_base_size(p)) return 0;

    trail = cpkt_flex_size(p);

    inc = types[cpkt_get_type(p)].inc;
    if (inc > 0) {
        my_len = trail / inc;
        trail %= inc;
    }

    // Is it an odd size?
    if (trail != 0) return 0;

    if (len) *len = my_len;

    return p;
}

/* This function uses privately allocated static memory. Caution is advised. */
static uint32_t *
cpkt_next_key(uint32_t *prev)
{
    static uint32_t next[3];

    sha512((char *) prev, 12, next, 12);
    return next;
}

static void
cpkt_add_nodes(peer p, struct cpkt_root_key_desc root_keys[], int len)
{
    manager mgr = p->manager;
    for (int i = 0; i < len; i++) {
        cpkt_root_key_desc rkd = root_keys + i;
        uint32_t *key = rkd->key;
        for (int j = 0; j < rkd->chain_len; j++, key = cpkt_next_key(key)) {
            node n = make_node_remote(key, p);
            if (!n) {
                warn("make_node_remote");
                continue;
            }
            manager_add_node(mgr, n);
        }
    }
}

// Assumes correct type.
static void
cpkt_ping_handle(cpkt cp, peer p)
{
    int len;
    cpkt_ping cp_ping = (cpkt_ping) cpkt_check_size(cp, &len);

    if (!cp_ping) return warnx("cp %p is not a ping packet", cp);

    cpkt_add_nodes(p, cp_ping->root_keys, len);

    peer_send_pong(p);
}

static void
cpkt_pong_handle(cpkt cp, peer p)
{
    int len;
    cpkt_pong cp_pong = (cpkt_pong) cpkt_check_size(cp, &len);
    if (!cp_pong) return warnx("cp %p is not a pong packet", cp);

    cpkt_add_nodes(p, cp_pong->root_keys, len);
}

static void
cpkt_link_handle(cpkt generic, peer p)
{
    cpkt_link c = (cpkt_link) cpkt_check_size(generic, 0);

    int r = manager_add_link(p->manager, c->key, p);
    if (r == -1) return; // just drop it -- the peer can retry if they want

    peer_send_linked(p, c->key);
}

static void
cpkt_linked_handle(cpkt generic, peer p)
{
    cpkt_linked c = (cpkt_linked) cpkt_check_size(generic, 0);

    prot_linked(p, c->key);
}

void
cpkt_handle(cpkt cp, peer p)
{
    if (cpkt_get_type(cp) >= KNOWN_CPKT_TYPE_CODES) {
        return warnx("ignoring message: unknown type %x", cpkt_get_type(cp));
    }

    cpkt_type t = types + cpkt_get_type(cp);

    if (!t->fn) {
        return warnx("ignoring %s (%x) message: unconfigured",
                cpkt_type_name(cp), cpkt_get_type(cp));
    }

    t->fn(cp, p);
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
make_cpkt_ping(in_addr_t addr, int cp_port, manager mgr)
{
    cpkt_ping cp = (cpkt_ping) make_cpkt(CPKT_BASE_SIZE(ping) +
            sizeof(struct cpkt_root_key_desc) * mgr->nbundles);
    if (!cp) return warnx("make_cpkt"), (cpkt) 0;

    cpkt_set_type((cpkt) cp, CPKT_TYPE_CODE_PING);

    cp->memcache_port = mgr->memcache_port;
    cp->http_port = mgr->http_port;
    cp->root_key_count = mgr->nbundles;
    for (int i = 0; i < mgr->nbundles; i++) {
        cp->root_keys[i].key[0] = mgr->all_bundles[i].storage->root_key[0];
        cp->root_keys[i].key[1] = mgr->all_bundles[i].storage->root_key[0];
        cp->root_keys[i].key[2] = mgr->all_bundles[i].storage->root_key[2];
        cp->root_keys[i].chain_len = mgr->all_bundles[i].storage->key_chain_len;
    }
    return (cpkt) cp;
}

cpkt
make_cpkt_pong(in_addr_t addr, uint16_t port, peer *peers, int len, manager mgr)
{
    cpkt_pong cp = (cpkt_pong) make_cpkt(CPKT_BASE_SIZE(pong) +
            sizeof(struct cpkt_root_key_desc) * mgr->nbundles);
    if (!cp) return warnx("make_cpkt"), (cpkt) 0;

    cpkt_set_type((cpkt) cp, CPKT_TYPE_CODE_PONG);

    if (len > PONG_PEER_MAX) len = PONG_PEER_MAX;
    for (int i = 0; i < len; i++) {
        cp->peers[i].addr = peers[i]->addr;
        cp->peers[i].port = peers[i]->cp_port;
    }
    for (int i = 0; i < mgr->nbundles; i++) {
        cp->root_keys[i].key[0] = mgr->all_bundles[i].storage->root_key[0];
        cp->root_keys[i].key[1] = mgr->all_bundles[i].storage->root_key[0];
        cp->root_keys[i].key[2] = mgr->all_bundles[i].storage->root_key[2];
        cp->root_keys[i].chain_len = mgr->all_bundles[i].storage->key_chain_len;
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

cpkt
make_cpkt_linked(uint32_t *key)
{
    cpkt_linked c = (cpkt_linked) make_cpkt(CPKT_BASE_SIZE(linked));
    if (!c) return warnx("make_cpkt"), (cpkt) 0;

    cpkt_set_type((cpkt) c, CPKT_TYPE_CODE_LINKED);
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
