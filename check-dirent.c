#include <stdlib.h>
#include <stdio.h>

#include "dirent.h"
#include "cut.h"

static struct key k0s = { 0x6b86b273ff34fce1LLU, 0x9d6b804eff5a3f57LLU },
                  k1s = { 0xd4735e3a265e16eeLLU, 0xe03f59718b9b5d03LLU };

static key k0 = &k0s, k1 = &k1s;

static dirent ds, dr;

void
__CUT_BRINGUP__null_dirent()
{
}

void
__CUT__null_dirent_doesnt_match()
{
    ASSERT(!dirent_matches(0, k1), "0 should not match key k1");
}

void
__CUT_TAKEDOWN__null_dirent()
{
}

void
__CUT_BRINGUP__stored_dirent()
{
    ds = make_dirent_stored(k0);
    ASSERT(!!ds, "Just trying to allocate here");
}

void
__CUT__stored_dirent_matches()
{
    ASSERT(dirent_matches(ds, k0), "ds should match key k0");
}

void
__CUT__stored_dirent_doesnt_match()
{
    ASSERT(!dirent_matches(ds, k1), "ds should not match key k1");
}

void
__CUT_TAKEDOWN__stored_dirent()
{
}

void
__CUT_BRINGUP__remote_dirent()
{
    dr = make_dirent_remote(k0);
    ASSERT(!!dr, "Just trying to allocate here");
}

void
__CUT__remote_dirent_matches()
{
    ASSERT(dirent_matches(dr, k0), "ds should match key k0");
}

void
__CUT__remote_dirent_doesnt_match()
{
    ASSERT(!dirent_matches(dr, k1), "ds should not match key k1");
}

void
__CUT_TAKEDOWN__remote_dirent()
{
}

