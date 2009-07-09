// key.c -- Math for keys

#include "key.h"
#include "util.h"

#define KEY_LEN 12

/* Keys must be little-endian. */
int
key_distance_cmp(uint32_t *x, uint32_t *a, uint32_t *b)
{
    uint8_t *X = (uint8_t *) x;
    uint8_t *A = (uint8_t *) a;
    uint8_t *B = (uint8_t *) b;

    for (int i = 0; i < KEY_LEN; i++) {
        uint8_t da = X[i] ^ A[i];
        uint8_t db = X[i] ^ B[i];
        if (da < db) return -1;
        if (da > db) return 1;
    }
    return 0;
}

