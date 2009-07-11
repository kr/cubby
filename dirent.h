// dirent.h -- Directory Entry header

#ifndef dirent_h
#define dirent_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct dirent *dirent;

#include "blob.h"
#include "region.h"

// bit masks/flags
#define RDESC_LOCAL 1
#define RDESC_REMOTE 2

// placeholder
typedef struct rdesc {
    char flags; // bit field
    char pad;
    uint16_t a;
    uint32_t b;
} *rdesc;

typedef struct rdesc_local {
    char flags; // bit field
    char pad;
    uint16_t reg;
    uint32_t off;
} *rdesc_local;

typedef struct rdesc_remote {
    char flags; // bit field
    char pad;
    uint16_t port;
    uint32_t addr;
} *rdesc_remote;

struct dirent {
    uint32_t key[3];
    char pad0[3];
    uint8_t len;
    struct rdesc rdescs[];
};

extern dirent invalid_dirent;

/* LEN is the number of rdesc entries to fit in this dirent. */
dirent make_dirent(uint32_t *key, uint8_t len);

/* NEW_LEN is the number of rdesc entries to fit in this dirent. */
dirent copy_dirent(dirent d, uint8_t new_len);

int dirent_matches(dirent d, uint32_t *key);

void dirent_set_rdesc_local(dirent d, int i, region r, blob b);
rdesc_local dirent_get_rdesc_local(dirent d);

int dirent_has_remote(dirent d, in_addr_t addr, uint16_t port);
int dirent_add_remote(dirent d, in_addr_t addr, uint16_t port);

#endif //dirent_h
