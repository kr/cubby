// region.h -- Contiguous space inside a bundle (header)

#ifndef region_h
#define region_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

#define REGION_BITS 19
//#define REGION_BITS 32

/* This is the on-disk structure for a region. */
/* PERFORMANCE NOTE: accessing members of this structure is likely to cause
   page faults and disk seeks */
/* If you modify this structure, you must increment BUNDLE_VERSION in bundle.h.
 */
typedef struct region_storage {
    uint32_t flags;
    char pad[7 * 8 + 4]; // reserved for future use
    char blobs[];
} *region_storage;

/* In-memory descriptor for a region */
typedef struct region {
    uint64_t size;
    region_storage storage;
} *region;

int regions_init(uint16_t count);

#endif //region_h
