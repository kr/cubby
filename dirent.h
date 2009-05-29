// dirent.h -- Directory Entry header

#ifndef dirent_h
#define dirent_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

// placeholder
typedef struct rdesc {
} *rdesc;

typedef struct dirent {
    uint32_t key[3];
    char pad0[3];
    uint8_t len;
    struct rdesc rdescs[];
} *dirent;

extern dirent invalid_dirent;

dirent make_dirent(uint32_t *key, uint8_t len);

int dirent_matches(dirent d, uint32_t *key);

#endif //dirent_h
