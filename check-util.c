#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "cut.h"

void
__CUT_BRINGUP__util()
{
}

void
__CUT__util_abcdef_startswith_abc()
{
    ASSERT(startswith("abcdef", "abc"), "abcdef starts with abc");
}

void
__CUT__util_not_abcdef_startswith_def()
{
    ASSERT(!startswith("abcdef", "def"), "abcdef does not start with def");
}

void
__CUT__util_abcdef_startswith_nul()
{
    ASSERT(startswith("abcdef", ""), "abcdef starts with \"\"");
}

void
__CUT__util_not_null_startswith_abc()
{
    ASSERT(!startswith(0, "abc"), "abcdef starts with \"\"");
}

void
__CUT__util_not_abcdef_startswith_null()
{
    ASSERT(!startswith("abcdef", 0), "abcdef starts with \"\"");
}

void
__CUT_TAKEDOWN__util()
{
}
