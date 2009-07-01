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
__CUT__util_int_from_timeval(void)
{
    struct timeval tv = { 345, 678 };
    uint64_t exp = 345000678;
    uint64_t got = int_from_timeval(&tv);

    ASSERT(exp == got, "expected %llu, got %llu", exp, got);
}

void
__CUT__util_timeval_from_int(void)
{
    uint64_t t = 345000678;
    struct timeval exp = { 345, 678 };
    struct timeval got = {};

    timeval_from_int(&got, t);

    ASSERT(exp.tv_sec == got.tv_sec, "expected %ld, got %ld",
            exp.tv_sec, got.tv_sec);
    ASSERT(exp.tv_usec == got.tv_usec, "expected %ld, got %ld",
            exp.tv_usec, got.tv_usec);
}

void
__CUT_TAKEDOWN__util()
{
}
