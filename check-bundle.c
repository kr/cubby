#include <stdio.h>
#include <string.h>

#include "cut.h"
#include "bundle.h"

void
__CUT_BRINGUP__void_bundle()
{
}

void
__CUT__void_bundle_test_make_root_key()
{
    int r = 0;
    uint32_t key[3] = { 0, };

    r = bundle_make_root_key(key);
    ASSERT(r != -1, "");
    ASSERT(key[0], "should contain random bits");
    ASSERT(key[1], "should contain random bits");
    ASSERT(key[2], "should contain random bits");
}

void
__CUT_TAKEDOWN__void_bundle()
{
}

