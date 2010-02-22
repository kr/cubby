#include <stdio.h>
#include <string.h>

#include "cut.h"
#include "key.h"
#include "sha512.h"

static uint32_t x[3] = { 0x6b86b273LU, 0xff34fce1LU, 0x9d6b804eLU },
                y[3] = { 0xd4735e3aLU, 0x265e16eeLU, 0xe03f5971LU };

void
__CUT_BRINGUP__void_key()
{
}

void
__CUT__void_key_test_make_key()
{
    ASSERT(1, "");
}

void
__CUT__void_key_test_key_fmt()
{
    char input[] = "Zurich";
    uint32_t key[3];
    char exp[] = "68ab5bb5.fd94ff11.9860c431";
    char got[27] = {};

    sha512(input, strlen(input), key, 12);
    key_fmt(got, key);
    ASSERT(!strcmp(exp, got), "got %s", got);
}

void
__CUT__void_key_test_key_cmp_a()
{
    uint32_t c[3];

    c[0] = x[0];
    c[1] = x[1];
    c[2] = x[2];
    c[2]++;

    int r = key_distance_cmp(c, x, y);
    ASSERT(r == -1, "x should be closer");
}

void
__CUT__void_key_test_key_cmp_b()
{
    uint32_t c[3];

    c[0] = y[0];
    c[1] = y[1];
    c[2] = y[2];
    c[2]++;

    int r = key_distance_cmp(c, x, y);
    ASSERT(r == 1, "y should be closer");
}

void
__CUT_TAKEDOWN__void_key()
{
}

