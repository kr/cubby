#include <stdlib.h>
#include <stdio.h>

#include "manager.h"
#include "cut.h"

static struct manager mgr = {};

void
__CUT_BRINGUP__manager()
{
}

void
__CUT__manager_with_no_free_regions()
{
    ASSERT(!mgr.free_regions_head, "should be empty");
    ASSERT(!mgr.free_regions_tail, "should be empty");
}

void
__CUT__manager_add_one_free_region()
{
    struct region r1 = {};

    manager_add_free_region(&mgr, &r1);
    ASSERT(mgr.free_regions_head == &r1, "should be just r1");
    ASSERT(mgr.free_regions_tail == &r1, "should be just r1");
    ASSERT(r1.next == 0, "should be just r1");
}

void
__CUT__manager_add_two_free_regions()
{
    struct region r1 = {}, r2 = {};

    manager_add_free_region(&mgr, &r1);
    manager_add_free_region(&mgr, &r2);

    ASSERT(mgr.free_regions_head == &r1, "should be r1, r2");
    ASSERT(mgr.free_regions_tail == &r2, "should be r1, r2");
    ASSERT(r1.next == &r2, "should be r1, r2");
    ASSERT(r2.next == 0, "should be r1, r2");
}

void
__CUT_TAKEDOWN__manager()
{
}

