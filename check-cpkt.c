#include <stdlib.h>
#include <stdio.h>

#include "cpkt.h"
#include "cut.h"

enum cpkt_type_codes {
    CPKT_TYPE_PING,
    CPKT_TYPE_PONG,
    CPKT_TYPE_LINK,
};

static cpkt p;

void
__CUT_BRINGUP__cpkt_void()
{
}

void
__CUT__cpkt_void_make()
{
    p = make_cpkt(50);
    ASSERT(p, "");
    ASSERT(p->size == 50, "");
}

void
__CUT__cpkt_void_handle_bad_ping()
{
    cpkt q;

    p = make_cpkt(50);
    cpkt_set_type(p, CPKT_TYPE_PING);

    ASSERT(cpkt_base_size(p) == 16, "got %d", cpkt_base_size(p));
    ASSERT(cpkt_flex_size(p) == 34, "got %d", cpkt_flex_size(p));

    q = cpkt_check_size(p);
    ASSERT(!q, "size should be invalid");

    //cpkt_ping_handle(p);
    //ASSERT(0, "");
}

void
__CUT_TAKEDOWN__cpkt_void()
{
}

