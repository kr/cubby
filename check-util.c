#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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
__CUT__util_usec_from_timeval(void)
{
    struct timeval tv = { 345, 678 };
    usec exp = 345000678;
    usec got = usec_from_timeval(&tv);

    ASSERT(exp == got, "expected %llu, got %llu", exp, got);
}

void
__CUT__util_timeval_from_usec(void)
{
    usec t = 345000678;
    struct timeval exp = { 345, 678 };
    struct timeval got = {};

    timeval_from_usec(&got, t);

    ASSERT(exp.tv_sec == got.tv_sec, "expected %ld, got %ld",
            exp.tv_sec, got.tv_sec);
    ASSERT(exp.tv_usec == got.tv_usec, "expected %ld, got %ld",
            exp.tv_usec, got.tv_usec);
}

void
__CUT__util_compute_key(void)
{
    uint32_t got[3] = {};
    uint32_t exp[3] = { 0x81fb346d, 0x17368bd9, 0xd5dd941e };

    key_for_peer(got, inet_addr("127.0.0.1"), htons(20202));

    ASSERT(got[0] == exp[0], "expected 0x%x, got 0x%x", exp[0], got[0]);
    ASSERT(got[1] == exp[1], "expected 0x%x, got 0x%x", exp[1], got[1]);
    ASSERT(got[2] == exp[2], "expected 0x%x, got 0x%x", exp[2], got[2]);
}

void
__CUT_TAKEDOWN__util()
{
}
