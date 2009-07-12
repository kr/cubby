// key.h -- Math for keys (header)

#ifndef key_h
#define key_h

#include "config.h"
#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

/* return -1 if key_distance(x, a) < key_distance(x, b);
   return 1 if key_distance(x, a) > key_distance(x, b);
   otherwise, return 0 */
int key_distance_cmp(uint32_t *x, uint32_t *a, uint32_t *b);

void key_fmt(char *out, uint32_t *key);

#define key_eq(a,b) ((a)[0] == (b)[0] && (a)[1] == (b)[1] && (a)[2] == (b)[2])

#endif //key_h
