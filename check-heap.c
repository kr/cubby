#include <stdlib.h>
#include <stdio.h>

#include "cut.h"
#include "heap.h"

static struct heap hh;
static heap hp = &hh;
static int x1 = 1, x2 = 2, x3 = 3;

int
check_cmp(int *a, int *b)
{
    if (*a > *b) return 1;
    if (*a < *b) return -1;
    return 0;
}

void
__CUT_BRINGUP__heap()
{
    heap_init(hp, (cmp_fn) check_cmp);
}

void
__CUT__heap_test_empty_heap_should_have_no_items()
{
    ASSERT(hp->used == 0, "hp should be empty.");
}

void
__CUT__heap_test_insert_one()
{
    heap_give(hp, &x1);
    ASSERT(hp->used == 1, "hp should contain one item.");
}

void
__CUT__heap_test_insert_and_remove_one()
{
    int r;
    int *x;

    r = heap_give(hp, &x1);
    ASSERT(r, "insert should succeed");

    x = heap_take(hp);
    ASSERT(x == &x1, "x1 should come back out");
    ASSERT(hp->used == 0, "hp should be empty.");
}

void
__CUT__heap_test_order()
{
    int r;
    int *x;

    r = heap_give(hp, &x2);
    ASSERT(r, "insert should succeed");

    r = heap_give(hp, &x3);
    ASSERT(r, "insert should succeed");

    r = heap_give(hp, &x1);
    ASSERT(r, "insert should succeed");

    x = heap_take(hp);
    ASSERT(x == &x1, "x1 should come out first.");

    x = heap_take(hp);
    ASSERT(x == &x2, "x2 should come out second.");

    x = heap_take(hp);
    ASSERT(x == &x3, "x3 should come out third.");
}

#define HOW_MANY 200
void
__CUT__heap_test_many_values()
{
    int r, i, *x, last = 0;
    int vals[HOW_MANY];

    for (i = 0; i < HOW_MANY; i++) {
        vals[i] = rand() % 8192;
        r = heap_give(hp, &vals[i]);
        ASSERT(r, "insert should succeed");
    }

    for (i = 0; i < HOW_MANY; i++) {
        x = heap_take(hp);
        ASSERT(*x >= last, "should come out in order.");
        last = *x;
    }
}

void
__CUT_TAKEDOWN__heap()
{
}

