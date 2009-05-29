// spht.h -- Slow but memory-efficient sparse hashtable header.

#ifndef spht_h
#define spht_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

#include <sys/types.h>

#include "dirent.h"
#include "sparr.h"

typedef struct spht {
    size_t ndel;
    size_t enlarge_threshold;
    sparr table;
} *spht;


spht make_spht(size_t cap);
dirent spht_get(spht h, uint32_t *k);
int spht_set(spht h, uint32_t *k, dirent v);
void spht_rm(spht h, uint32_t *k);

size_t spht_fill(spht h);

#endif //spht_h
