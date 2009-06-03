// dirent.h -- Directory Entry header

#ifndef dirent_h
#define dirent_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

typedef struct dirent *dirent;

#include "blob.h"
#include "region.h"

#define RDESC_LOCAL 1

// placeholder
typedef struct rdesc {
    char flags;
    char pad[7];
} *rdesc;

typedef struct rdesc_local {
    char flags;
    char pad;
    uint16_t reg;
    uint32_t off;
} *rdesc_local;

struct dirent {
    uint32_t key[3];
    char pad0[3];
    uint8_t len;
    struct rdesc rdescs[];
};

extern dirent invalid_dirent;

dirent make_dirent(uint32_t *key, uint8_t len);

int dirent_matches(dirent d, uint32_t *key);

void dirent_set_rdesc_local(dirent d, int i, region r, blob b);

#endif //dirent_h
