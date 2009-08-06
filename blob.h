// blob.h -- An individual blob on disk (header)

#ifndef blob_h
#define blob_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

/* This is the on-disk structure for a blob. */
/* PERFORMANCE NOTE: accessing members of this structure is likely to cause
   page faults and disk seeks */
/* If you modify this structure, you must increment BUNDLE_VERSION in bundle.h.
 */
typedef struct blob {
    uint32_t size; // size == 0 means end-of-region
    uint32_t key[3];
    uint8_t num_copies;
    char pad[7 + 8]; // reserved
    char data[];
} *blob;

blob blob_next(blob bl);
int blob_verify(blob bl);

#endif //blob_h
