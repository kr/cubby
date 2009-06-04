#include <stdlib.h>
#include <stdio.h>

#include "region.h"
#include "cut.h"

#define BUFFER_SIZE 1024
#define REG_HEADER_SIZE (sizeof(struct region_storage))
#define BLOB_HEADER_SIZE (sizeof(struct blob))
#define CAP (BUFFER_SIZE - REG_HEADER_SIZE)
#define MAX_BLOB (CAP - BLOB_HEADER_SIZE)

static char reg_storage[BUFFER_SIZE] = {};
static struct region reg = {};

void
__CUT_BRINGUP__void_region()
{
}

void
__CUT__void_region_constructor()
{
#define ID 7

    region_init(&reg, ID, (region_storage) reg_storage, sizeof(reg_storage));

    ASSERT(reg.id == ID, "");
    ASSERT(reg.storage == (region_storage) reg_storage, "");
    ASSERT(reg.free == reg.storage->blobs, "");
    ASSERT(reg.top == reg_storage + sizeof(reg_storage), "");
    ASSERT(reg.next == 0, "");
}

void
__CUT_TAKEDOWN__void_region()
{
}

void
__CUT_BRINGUP__empty_region()
{
    region_init(&reg, 0, (region_storage) reg_storage, sizeof(reg_storage));
}

void
__CUT__empty_region_header_size()
{
    ASSERT(REG_HEADER_SIZE == reg.storage->blobs - (char *) reg.storage, "");
}

void
__CUT__empty_region_should_have_space()
{
    ASSERT(region_has_space_for_blob(&reg, MAX_BLOB), "should have space");
}

void
__CUT__empty_region_should_not_have_space()
{
    ASSERT(!region_has_space_for_blob(&reg, MAX_BLOB + 1),
           "should not have space");
}

void
__CUT__empty_region_allocate_max_blob()
{
    ASSERT(region_allocate_blob(&reg, MAX_BLOB), "should have space");
}

void
__CUT__empty_region_allocate_max_blob_plus_one()
{
    ASSERT(!region_allocate_blob(&reg, MAX_BLOB + 1), "should not have space");
}

void
__CUT_TAKEDOWN__empty_region()
{
}

void
__CUT_BRINGUP__full_region()
{
    region_init(&reg, 0, (region_storage) reg_storage, sizeof(reg_storage));
    region_allocate_blob(&reg, MAX_BLOB);
}

void
__CUT__full_region_should_have_no_space()
{
    ASSERT(!region_has_space_for_blob(&reg, 0), "should not have space");
}

void
__CUT__full_region_allocate_blob()
{
    ASSERT(!region_allocate_blob(&reg, 0), "should not have space");
}

void
__CUT_TAKEDOWN__full_region()
{
}

