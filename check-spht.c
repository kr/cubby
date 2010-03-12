#include <stdlib.h>
#include <stdio.h>

#include "spht.h"
#include "cut.h"

static spht h;

static uint32_t k0[3] = { 0x6b86b273LU, 0xff34fce1LU, 0x9d6b804eLU },
                k1[3] = { 0xd4735e3aLU, 0x265e16eeLU, 0xe03f5971LU },
                k2[3] = { 0x00000000LU, 0x00000002LU, 0x00000000LU },
                k3[3] = { 0x00000000LU, 0x00000003LU, 0x00000000LU },
                k4[3] = { 0x00000000LU, 0x00000004LU, 0x00000000LU },
                k5[3] = { 0x00000000LU, 0x00000005LU, 0x00000000LU };

static dirent dr0, dr1, dr2, dr3, dr4, dr5;

void
__CUT_BRINGUP__new_spht()
{
}

void
__CUT__new_spht_0_should_be_min_cap()
{
    h = make_spht(0);

    ASSERT(!!h, "should be able to allocate");
    ASSERT(h->table->cap == 4, "min cap is 4");
}

void
__CUT__new_spht_5_should_be_power_of_two()
{
    h = make_spht(5);

    ASSERT(!!h, "should be able to allocate");
    ASSERT((h->table->cap & (h->table->cap - 1)) == 0, "must be power of two");
}

void
__CUT_TAKEDOWN__new_spht()
{
}

void
__CUT_BRINGUP__empty_spht()
{
    h = make_spht(3);
    ASSERT(!!h, "Just trying to allocate here");
    dr0 = make_dirent(k0, 0, 0);
    ASSERT(!!dr0, "Just trying to allocate here");
    dr1 = make_dirent(k1, 0, 0);
    ASSERT(!!dr1, "Just trying to allocate here");
    dr2 = make_dirent(k2, 0, 0);
    ASSERT(!!dr1, "Just trying to allocate here");
    dr3 = make_dirent(k3, 0, 0);
    ASSERT(!!dr1, "Just trying to allocate here");
    dr4 = make_dirent(k4, 0, 0);
    ASSERT(!!dr1, "Just trying to allocate here");
    dr5 = make_dirent(k5, 0, 0);
    ASSERT(!!dr1, "Just trying to allocate here");
}

void
__CUT__empty_spht_should_be_empty()
{
    ASSERT(spht_fill(h) == 0, "h should contain zero things");
    ASSERT(!spht_get(h, k0), "h[k0] should be empty");
    ASSERT(!spht_get(h, k1), "h[k1] should be empty");
}

void
__CUT__empty_spht_setting_k0_should_work()
{
    int r;

    r = spht_set(h, dr0);

    ASSERT(!r, "should claim success");
    ASSERT(spht_fill(h) == 1, "h should contain one thing");
    ASSERT(spht_get(h, k0) == dr0, "h[k0] should be dr0");
}

void
__CUT__empty_spht_replace()
{
    int r;

    dirent dr0b = make_dirent(k0, 0, 0);
    ASSERT(!!dr0b, "Just trying to allocate here");

    r = spht_set(h, dr0);
    ASSERT(!r, "should claim success");
    ASSERT(spht_fill(h) == 1, "h should contain one thing");
    ASSERT(spht_get(h, k0) == dr0, "h[k0] should be dr0");

    r = spht_set(h, dr0b);
    ASSERT(!r, "should claim success");
    ASSERT(spht_fill(h) == 1, "h should contain one thing");
    ASSERT(spht_get(h, k0) == dr0b, "h[k0] should be dr0b");
}

void
__CUT__empty_spht_setting_k0_should_leave_others()
{
    int r;

    r = spht_set(h, dr0);

    ASSERT(!r, "should claim success");
    ASSERT(!spht_get(h, k1), "h[k1] should be empty");
}

void
__CUT__empty_spht_deleting_k0_should_nop()
{
    spht_rm(h, k1);

    ASSERT(spht_fill(h) == 0, "h should contain no thing");
    ASSERT(!spht_get(h, k1), "h[k1] should be empty");
}

void
__CUT_TAKEDOWN__empty_spht()
{
}

void
__CUT_BRINGUP__full_spht()
{
    int r;

    h = make_spht(3);
    ASSERT(!!h, "Just trying to allocate here");

    dr0 = make_dirent(k0, 0, 0);
    ASSERT(!!dr0, "Just trying to allocate here");
    dr1 = make_dirent(k1, 0, 0);
    ASSERT(!!dr1, "Just trying to allocate here");
    dr2 = make_dirent(k2, 0, 0);
    ASSERT(!!dr1, "Just trying to allocate here");
    dr3 = make_dirent(k3, 0, 0);
    ASSERT(!!dr1, "Just trying to allocate here");
    dr4 = make_dirent(k4, 0, 0);
    ASSERT(!!dr1, "Just trying to allocate here");
    dr5 = make_dirent(k5, 0, 0);
    ASSERT(!!dr1, "Just trying to allocate here");

    r = spht_set(h, dr0);
    ASSERT(!r, "Just trying to allocate here");
    r = spht_set(h, dr1);
    ASSERT(!r, "Just trying to allocate here");
}

void
__CUT__full_spht_should_be_full()
{
    ASSERT(spht_fill(h) == 2, "h should contain two things");
    ASSERT(sparr_get(h->table, 3) == dr0, "h->table[3] should be dr0");
    ASSERT(spht_get(h, k0) == dr0, "h[k0] should be dr0");
    ASSERT(spht_get(h, k1) == dr1, "h[k1] should be dr1");
}

void
__CUT__full_spht_deleting_k0_should_work()
{
    spht_rm(h, k1);

    ASSERT(spht_fill(h) == 1, "h should contain one thing");
    ASSERT(!spht_get(h, k1), "h[k1] should be empty");
}

void
__CUT__full_spht_deleting_k0_should_leave_others()
{
    spht_rm(h, k1);

    ASSERT(spht_get(h, k0) == dr0, "h[k0] should be dr0");
}

void
__CUT__full_spht_setting_null()
{
    int r;

    r = spht_set(h, (dirent) 0);

    ASSERT(r == -1, "should claim failure");
}

void
__CUT__full_spht_grows()
{
    size_t old_cap;

    ASSERT(spht_fill(h) == 2, "we start out with 2 items");
    ASSERT(h->enlarge_threshold == 3, "should enlarge after 3 items");

    old_cap = h->table->cap;
    spht_set(h, dr2);
    ASSERT(old_cap == h->table->cap, "the table should not have grown yet");
    spht_set(h, dr3);
    ASSERT(old_cap < h->table->cap, "the table should have grown");
}

void
__CUT_TAKEDOWN__full_spht()
{
}

