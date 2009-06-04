// region.h -- Contiguous space inside a bundle (header)

#ifndef region_h
#define region_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

typedef struct region_storage *region_storage;
typedef struct region *region;

#include "spht.h"
#include "blob.h"
#include "manager.h"

#define REGION_BITS 19
//#define REGION_BITS 32

/* This is the on-disk structure for a region. */
/* PERFORMANCE NOTE: accessing members of this structure is likely to cause
   page faults and disk seeks */
/* If you modify this structure, you must increment BUNDLE_VERSION in bundle.h.
 */
struct region_storage {
    uint32_t flags;
    char pad[7 * 8 + 4]; // reserved for future use
    char blobs[];
};

/* In-memory descriptor for a region */
struct region {
    char *top;
    char *free;
    uint16_t id;
    region_storage storage;
    region next;
};

int region_has_space_for_blob(region r, size_t n);

/* Reserves space for size bytes in region r. Does not touch the disk. */
blob region_allocate_blob(region r, size_t size);

void region_delete_blob(region r, blob b);

void region_init(region r, int id, region_storage storage, size_t size);

size_t region_blob_offset(region r, blob b);

blob region_get_blob_by_off(region r, uint32_t off);

int region_space_cmp(region a, region b);

#endif //region_h
