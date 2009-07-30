// blob.h -- An individual blob on disk (header)

#ifndef blob_h
#define blob_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

typedef struct blob {
    uint32_t key[3];
    uint32_t size;
    uint8_t num_copies;
    char pad[7 + 8]; // reserved
    char data[];
} *blob;

blob blob_next(blob bl);
int blob_verify(blob bl);

#endif //blob_h
