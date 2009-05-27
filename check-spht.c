#include <stdlib.h>
#include <stdio.h>

#include "spht.h"
#include "cut.h"

static spht h;

static struct key k0s = { 0x6b86b273ff34fce1LLU, 0x9d6b804eff5a3f57LLU },
                  k1s = { 0xd4735e3a265e16eeLLU, 0xe03f59718b9b5d03LLU };

static key k0 = &k0s, k1 = &k1s;

static dirent dr0, dr1;

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
    dr0 = make_dirent_remote(k0);
    ASSERT(!!dr0, "Just trying to allocate here");
    dr1 = make_dirent_remote(k1);
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

    r = spht_set(h, k0, dr0);

    ASSERT(!r, "should claim success");
    ASSERT(spht_fill(h) == 1, "h should contain one thing");
    ASSERT(spht_get(h, k0) == dr0, "h[k0] should be dr0");
}

void
__CUT__empty_spht_setting_k0_should_leave_others()
{
    int r;

    r = spht_set(h, k0, dr0);

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

    dr0 = make_dirent_remote(k0);
    ASSERT(!!dr0, "Just trying to allocate here");
    dr1 = make_dirent_remote(k1);
    ASSERT(!!dr1, "Just trying to allocate here");

    r = spht_set(h, k0, dr0);
    ASSERT(!r, "Just trying to allocate here");
    r = spht_set(h, k1, dr1);
    ASSERT(!r, "Just trying to allocate here");
}

void
__CUT__full_spht_should_be_full()
{
    ASSERT(spht_fill(h) == 2, "h should contain two things");
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
__CUT__full_spht_setting_k0_to_0()
{
    int r;

    r = spht_set(h, k1, (dirent) 0);

    ASSERT(!r, "should claim success");
    ASSERT(spht_fill(h) == 1, "h should contain one thing");
    ASSERT(!spht_get(h, k1), "h[k1] should be empty");
}

void
__CUT_TAKEDOWN__full_spht()
{
}

