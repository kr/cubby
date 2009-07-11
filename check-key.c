#include <stdio.h>
#include <string.h>

#include "cut.h"
#include "key.h"
#include "sha512.h"

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
__CUT_TAKEDOWN__void_key()
{
}

