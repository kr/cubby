#include <stdlib.h>
#include <stdio.h>

#include "dirent.h"
#include "cut.h"

static uint32_t k0[3] = { 0x6b86b273LU, 0xff34fce1LU, 0x9d6b804eLU },
                k1[3] = { 0xd4735e3aLU, 0x265e16eeLU, 0xe03f5971LU };

static dirent d0;

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
    d0 = make_dirent(k0, 0);
    ASSERT(!!d0, "Just trying to allocate here");
}

void
__CUT__stored_dirent_matches()
{
    ASSERT(dirent_matches(d0, k0), "d0 should match key k0");
}

void
__CUT__stored_dirent_doesnt_match()
{
    ASSERT(!dirent_matches(d0, k1), "d0 should not match key k1");
}

void
__CUT_TAKEDOWN__stored_dirent()
{
}
