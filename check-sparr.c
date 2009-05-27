#include <stdlib.h>
#include <stdio.h>

#include "sparr.h"
#include "cut.h"

static sparr a;
static spgroup g;

void
__CUT_BRINGUP__empty_spgroup()
{
    g = make_spgroup();
    ASSERT(!!g, "Just trying to allocate here");
}

void
__CUT__empty_spgroup_should_be_empty()
{
    ASSERT(g->fill == 0, "g should contain zero things");
    ASSERT(!spgroup_get(g, 0), "g[0] should be empty");
    ASSERT(!spgroup_test(g, 0), "g[0] should be empty");
    ASSERT(!spgroup_get(g, 1), "g[1] should be empty");
    ASSERT(!spgroup_test(g, 1), "g[1] should be empty");
    ASSERT(!spgroup_get(g, 2), "g[2] should be empty");
    ASSERT(!spgroup_test(g, 2), "g[2] should be empty");
    ASSERT(!spgroup_get(g, GROUP_SIZE - 1), "g[GROUP_SIZE-1] should be empty");
    ASSERT(!spgroup_test(g, GROUP_SIZE - 1),
           "g[GROUP_SIZE - 1] should be empty");
}

void
__CUT__empty_spgroup_setting_first_should_work()
{
    int r;

    r = spgroup_set(g, 0, (dirent) 7);

    ASSERT(!r, "should claim success");
    ASSERT(g->fill == 1, "g should contain one thing");
    ASSERT(((int) spgroup_get(g, 0)) == 7, "g[0] should be 7");
    ASSERT(spgroup_test(g, 0), "g[0] should be full");
}

void
__CUT__empty_spgroup_setting_first_should_leave_others()
{
    int r;

    r = spgroup_set(g, 0, (dirent) 7);

    ASSERT(!r, "should claim success");
    ASSERT(!spgroup_get(g, 1), "g[1] should be empty");
    ASSERT(!spgroup_test(g, 1), "g[1] should be empty");
    ASSERT(!spgroup_get(g, 2), "g[2] should be empty");
    ASSERT(!spgroup_test(g, 2), "g[2] should be empty");
    ASSERT(!spgroup_get(g, GROUP_SIZE - 1), "g[GROUP_SIZE-1] should be empty");
    ASSERT(!spgroup_test(g, GROUP_SIZE - 1),
           "g[GROUP_SIZE - 1] should be empty");
}

void
__CUT__empty_spgroup_deleting_first_should_nop()
{
    int r;

    r = spgroup_rm(g, 0);

    ASSERT(!r, "should claim success");
    ASSERT(g->fill == 0, "g should contain no thing");
    ASSERT(!spgroup_get(g, 0), "g[0] should be empty");
    ASSERT(!spgroup_test(g, 0), "g[0] should be empty");
}

void
__CUT_TAKEDOWN__empty_spgroup()
{
}

void
__CUT_BRINGUP__full_spgroup()
{
    int r;

    g = make_spgroup(3);
    ASSERT(!!g, "Just trying to allocate here");
    r = spgroup_set(g, 0, (dirent) 7);
    ASSERT(!r, "Just trying to allocate here");
    r = spgroup_set(g, 1, (dirent) 11);
    ASSERT(!r, "Just trying to allocate here");
    r = spgroup_set(g, 2, (dirent) 13);
    ASSERT(!r, "Just trying to allocate here");
}

void
__CUT__full_spgroup_should_be_full()
{
    ASSERT(g->fill == 3, "g should contain three things");
    ASSERT(((int) spgroup_get(g, 0)) == 7, "g[0] should be 7");
    ASSERT(spgroup_test(g, 0), "g[0] should be full");
    ASSERT(((int) spgroup_get(g, 1)) == 11, "g[1] should be 11");
    ASSERT(spgroup_test(g, 1), "g[1] should be full");
    ASSERT(((int) spgroup_get(g, 2)) == 13, "g[2] should be 13");
    ASSERT(spgroup_test(g, 2), "g[2] should be full");
}

void
__CUT__full_spgroup_deleting_first_should_work()
{
    int r;

    r = spgroup_rm(g, 0);

    ASSERT(!r, "should claim success");
    ASSERT(g->fill == 2, "g should contain two things");
    ASSERT(!spgroup_get(g, 0), "g[0] should be empty");
    ASSERT(!spgroup_test(g, 0), "g[0] should be empty");
}

void
__CUT__full_spgroup_deleting_first_should_leave_others()
{
    int r;

    r = spgroup_rm(g, 0);

    ASSERT(!r, "should claim success");
    ASSERT(((int) spgroup_get(g, 1)) == 11, "g[1] should be 11");
    ASSERT(spgroup_test(g, 1), "g[1] should be full");
    ASSERT(((int) spgroup_get(g, 2)) == 13, "g[2] should be 13");
    ASSERT(spgroup_test(g, 2), "g[2] should be full");
}

void
__CUT__full_spgroup_setting_first_to_0()
{
    int r;

    r = spgroup_set(g, 0, (dirent) 0);

    ASSERT(!r, "should claim success");
    ASSERT(g->fill == 2, "g should contain two things");
    ASSERT(!spgroup_get(g, 0), "g[0] should be empty");
    ASSERT(!spgroup_test(g, 0), "g[0] should be empty");
}

void
__CUT_TAKEDOWN__full_spgroup()
{
}
void
__CUT_BRINGUP__empty_sparr()
{
    a = make_sparr(3);
    ASSERT(!!a, "Just trying to allocate here");
}

void
__CUT__empty_sparr_should_be_empty()
{
    ASSERT(a->fill == 0, "a should contain zero things");
    ASSERT(!sparr_get(a, 0), "a[0] should be empty");
    ASSERT(!sparr_test(a, 0), "g[0] should be empty");
    ASSERT(!sparr_get(a, 1), "a[1] should be empty");
    ASSERT(!sparr_test(a, 1), "g[1] should be empty");
    ASSERT(!sparr_get(a, 2), "a[2] should be empty");
    ASSERT(!sparr_test(a, 2), "g[2] should be empty");
}

void
__CUT__empty_sparr_setting_first_should_work()
{
    int r;

    r = sparr_set(a, 0, (dirent) 7);

    ASSERT(!r, "should claim success");
    ASSERT(a->fill == 1, "a should contain one thing");
    ASSERT(((int) sparr_get(a, 0)) == 7, "a[0] should be 7");
    ASSERT(sparr_test(a, 0), "g[0] should be full");
}

void
__CUT__empty_sparr_setting_first_should_leave_others()
{
    int r;

    r = sparr_set(a, 0, (dirent) 7);

    ASSERT(!r, "should claim success");
    ASSERT(!sparr_get(a, 1), "a[1] should be empty");
    ASSERT(!sparr_test(a, 1), "g[1] should be empty");
    ASSERT(!sparr_get(a, 2), "a[2] should be empty");
    ASSERT(!sparr_test(a, 2), "g[2] should be empty");
}

void
__CUT__empty_sparr_deleting_first_should_nop()
{
    int r;

    r = sparr_rm(a, 0);

    ASSERT(!r, "should claim success");
    ASSERT(a->fill == 0, "a should contain no thing");
    ASSERT(!sparr_get(a, 0), "a[0] should be empty");
    ASSERT(!sparr_test(a, 0), "g[0] should be empty");
}

void
__CUT_TAKEDOWN__empty_sparr()
{
}

void
__CUT_BRINGUP__full_sparr()
{
    int r;

    a = make_sparr(3);
    ASSERT(!!a, "Just trying to allocate here");
    r = sparr_set(a, 0, (dirent) 7);
    ASSERT(!r, "Just trying to allocate here");
    r = sparr_set(a, 1, (dirent) 11);
    ASSERT(!r, "Just trying to allocate here");
    r = sparr_set(a, 2, (dirent) 13);
    ASSERT(!r, "Just trying to allocate here");
}

void
__CUT__full_sparr_should_be_full()
{
    ASSERT(a->fill == 3, "a should contain three things");
    ASSERT(((int) sparr_get(a, 0)) == 7, "a[0] should be 7");
    ASSERT(sparr_test(a, 0), "g[0] should be full");
    ASSERT(((int) sparr_get(a, 1)) == 11, "a[1] should be 11");
    ASSERT(sparr_test(a, 1), "g[1] should be full");
    ASSERT(((int) sparr_get(a, 2)) == 13, "a[2] should be 13");
    ASSERT(sparr_test(a, 2), "g[2] should be full");
}

void
__CUT__full_sparr_deleting_first_should_work()
{
    int r;

    r = sparr_rm(a, 0);

    ASSERT(!r, "should claim success");
    ASSERT(a->fill == 2, "a should contain two things");
    ASSERT(!sparr_get(a, 0), "a[0] should be empty");
    ASSERT(!sparr_test(a, 0), "g[0] should be empty");
}

void
__CUT__full_sparr_deleting_first_should_leave_others()
{
    int r;

    r = sparr_rm(a, 0);

    ASSERT(!r, "should claim success");
    ASSERT(((int) sparr_get(a, 1)) == 11, "a[1] should be 11");
    ASSERT(sparr_test(a, 1), "g[1] should be full");
    ASSERT(((int) sparr_get(a, 2)) == 13, "a[2] should be 13");
    ASSERT(sparr_test(a, 2), "g[2] should be full");
}

void
__CUT__full_sparr_setting_first_to_0()
{
    int r;

    r = sparr_set(a, 0, (dirent) 0);

    ASSERT(!r, "should claim success");
    ASSERT(a->fill == 2, "a should contain two things");
    ASSERT(!sparr_get(a, 0), "a[0] should be empty");
    ASSERT(!sparr_test(a, 0), "g[0] should be empty");
}

void
__CUT_TAKEDOWN__full_sparr()
{
}

