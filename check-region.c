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
__CUT_BRINGUP__empty_region()
{
    // TODO refactor this into a region constructor
    reg.id = 0;
    reg.storage = (region_storage) reg_storage;
    reg.free = reg.storage->blobs;
    reg.top = reg_storage + sizeof(reg_storage);
    reg.next = 0;
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
}

void
__CUT__full_region_with_no_free_regions()
{
}

void
__CUT_TAKEDOWN__full_region()
{
}

