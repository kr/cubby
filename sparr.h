// sparr.h -- Slow but memory-efficient sparse array header.

#ifndef sparr_h
#define sparr_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

#include <sys/types.h>

typedef struct sparr *sparr;

#include "dirent.h"
#include "util.h"

#define GROUP_SIZE 48

typedef struct spgroup {
    uint8_t mask[CEILDIV(GROUP_SIZE, 8)];
    uint16_t fill;
    dirent *slots;
} *spgroup;

struct sparr {
    size_t cap; // Capacity of this array.
    size_t fill; // How many buckets are occupied.
    spgroup groups[];
};


spgroup make_spgroup();
int spgroup_test(spgroup g, uint16_t i);
dirent spgroup_get(spgroup g, uint16_t i);
int spgroup_set(spgroup g, uint16_t i, dirent v);
int spgroup_rm(spgroup g, uint16_t i);
void spgroup_free(spgroup g);

sparr make_sparr(int cap);
int sparr_test(sparr a, size_t i);
dirent sparr_get(sparr a, size_t i);
int sparr_set(sparr a, size_t i, dirent v);
int sparr_rm(sparr a, size_t i);
void sparr_free(sparr a);

#endif //sparr_h
