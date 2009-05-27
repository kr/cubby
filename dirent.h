// dirent.h -- Directory Entry header

#ifndef dirent_h
#define dirent_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

#define INVALID_DIRENT ((dirent) 4)

typedef struct key {
    uint64_t a, b;
} *key;

typedef struct dirent {
    uint8_t type;
    uint8_t pad[7];
    uint64_t key_hint;
} *dirent;

typedef struct dirent_stored {
    uint64_t location;
    uint64_t key_hint;
} *dirent_stored;

typedef struct dirent_remote {
    uint8_t type;
    uint8_t pad[3];
    uint32_t addr; // network order
    struct key k;
} *dirent_remote;

dirent make_dirent_stored(key k);
dirent make_dirent_remote(key k);

int dirent_matches(dirent d, key k);

#endif //dirent_h
