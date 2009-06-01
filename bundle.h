// bundle.h -- A disk file containing cubby data

#ifndef bundle_h
#define bundle_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

#include "region.h"

#define BUNDLE_VERSION 1

/* This is the on-disk structure for a bundle. */
/* PERFORMANCE NOTE: accessing members of this structure is likely to cause
   page faults and disk seeks */
/* If you modify this structure, you must increment BUNDLE_VERSION. */
typedef struct bundle_storage {
    uint32_t magic;
    uint32_t version;
    char pad[7 * 8]; // reserved for future use
    char regions[];
} *bundle_storage;

/* In-memory descriptor for a bundle */
typedef struct bundle {
    char *name;
    uint16_t nregions;
    uint64_t tot_size; // size of bundle including the header
    uint64_t reg_size; // size available for regions
    bundle_storage storage;
} *bundle;

extern int initialize_bundles;

int add_bundle(char *name);
void bundle_sync(bundle b, int sync);
int bundles_init();
uint16_t bundles_count();
bundle bundle_get(uint16_t i);
region_storage bundle_get_region_storage(bundle b, uint16_t i);

#endif //bundle_h
