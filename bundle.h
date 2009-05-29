// bundle.h -- A disk file that contains cubby data header

#ifndef bundle_h
#define bundle_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

typedef struct bundle {
    char *name;
    uint16_t nregions;
    void *base;
} *bundle;

int add_bundle(char *name);

#endif //bundle_h
