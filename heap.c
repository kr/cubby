/* heap.c - binary heap */

/* Copyright (C) 2007 Keith Rarick and Philotic Inc.

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>

#include "heap.h"

void
heap_init(heap hp, cmp_fn cmp)
{
    if (!hp) return;

    hp->cap = 0;
    hp->used = 0;
    hp->cmp = cmp;
    hp->heap = 0;

    return;
}

void
heap_clear(heap hp)
{
    free(hp->heap);
    heap_init(hp, hp->cmp);
}

static void
heap_grow(heap hp)
{
    void **nheap;
    unsigned int ncap = hp->cap << 1 ? : 1;

    nheap = malloc(ncap * sizeof(void *));
    if (!nheap) return;

    if (hp->heap) memcpy(nheap, hp->heap, hp->used * sizeof(void *));
    free(hp->heap);
    hp->heap = nheap;
    hp->cap = ncap;
}

static void
swap(heap hp, unsigned int a, unsigned int b)
{
    void *x;

    x = hp->heap[a];
    hp->heap[a] = hp->heap[b];
    hp->heap[b] = x;
}

#define PARENT(i) (((i-1))>>1)
#define CHILD_LEFT(i) (((i)<<1)+1)
#define CHILD_RIGHT(i) (((i)<<1)+2)

static int
cmp(heap hp, unsigned int a, unsigned int b)
{
    return hp->cmp(hp->heap[a], hp->heap[b]);
}

static void
bubble_up(heap hp, unsigned int k)
{
    int p;

    if (k == 0) return;
    p = PARENT(k);
    if (cmp(hp, p, k) <= 0) return;
    swap(hp, k, p);
    bubble_up(hp, p);
}

static void
bubble_down(heap hp, unsigned int k)
{
    int l, r, s;

    l = CHILD_LEFT(k);
    r = CHILD_RIGHT(k);

    s = k;
    if (l < hp->used && cmp(hp, l, k) < 0) s = l;
    if (r < hp->used && cmp(hp, r, s) < 0) s = r;
    if (s == k) return; /* already satisfies the heap property */

    swap(hp, k, s);
    bubble_down(hp, s);
}

/* assumes there is at least one item in the heap */
static void
delete_min(heap hp)
{
    hp->heap[0] = hp->heap[--hp->used];
    if (hp->used) bubble_down(hp, 0);
}

int
heap_give(heap hp, void *x)
{
    int k;

    if (hp->used >= hp->cap) heap_grow(hp);
    if (hp->used >= hp->cap) return 0;

    k = hp->used++;
    hp->heap[k] = x;
    bubble_up(hp, k);

    return 1;
}

void *
heap_take(heap hp)
{
    void *x;

    if (hp->used == 0) return 0;

    x = hp->heap[0];
    delete_min(hp);
    return x;
}

void *
heap_peek(heap hp)
{
    if (hp->used == 0) return 0;
    return hp->heap[0];
}

