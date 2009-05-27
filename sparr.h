// sparr.h -- Slow but memory-efficient sparse array header.

#ifndef sparr_h
#define sparr_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

#include <sys/types.h>

#define GROUP_SIZE 48

typedef struct spgroup {
    uint8_t mask[(GROUP_SIZE - 1) / 8 + 1]; // ceiling division
    uint16_t fill;
    void **slots;
} *spgroup;

typedef struct sparr {
    size_t cap; // Capacity of this array.
    size_t fill; // How many buckets are occupied.
    spgroup groups[];
} *sparr;


spgroup make_spgroup();
void *spgroup_get(spgroup g, uint16_t i);
int spgroup_set(spgroup g, uint16_t i, void *v);
int spgroup_rm(spgroup g, uint16_t i);

sparr make_sparr(int cap);
void *sparr_get(sparr a, size_t i);
int sparr_set(sparr a, size_t i, void *v);
int sparr_rm(sparr a, size_t i);

#endif //sparr_h
