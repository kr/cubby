// sparr.c -- Slow but memory-efficient sparse array.

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "sparr.h"
#include "util.h"

/*

This is exactly the same idea as what you'll find in Google's sparsehash
package, but without the STL and C++ diarrhea.

http://code.google.com/p/google-sparsehash/

*/

/* Bitmask stuff */

static uint8_t
charbit(uint8_t i)
{
    return i >> 3;
}

static uint8_t
modbit(uint8_t i)
{
    return 1 << (i & 7);
}

static int
bmtest(uint8_t *bm, uint16_t i)
{
    return bm[charbit(i)] & modbit(i);
}

static int
bmset(uint8_t *bm, uint16_t i)
{
    return bm[charbit(i)] |= modbit(i);
}

static int
bmclear(uint8_t *bm, uint16_t i)
{
    return bm[charbit(i)] &= ~modbit(i);
}

static uint16_t
bmcount(uint8_t *bm, uint16_t i)
{
    uint16_t c, res;
    static const char cbits[256] = {
        0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
        4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
    };

    for (c = i / 8; c; c--) res += cbits[*bm++];
    return res + cbits[*bm & ((1 << (i % 8)) - 1)];
}

/* Sparse Group */

spgroup
make_spgroup()
{
    spgroup g;

    g = malloc(sizeof(struct spgroup));
    if (!g) return warn("malloc"), (spgroup) 0;

    memset(g, 0, sizeof(*g));

    return g;
}

void *
spgroup_get(spgroup g, uint16_t i)
{
    if (!bmtest(g->mask, i)) return 0;

    return g->slots[bmcount(g->mask, i)];
}

#include <stdio.h>

int
spgroup_rm(spgroup g, uint16_t i)
{
    void **nslots;
    uint16_t slot;

    if (!bmtest(g->mask, i)) return 0;

    nslots = malloc(sizeof(void *) * (g->fill - 1));
    if (!nslots) return warn("realloc"), -1;

    slot = bmcount(g->mask, i);
    memcpy(nslots, g->slots, sizeof(void *) * slot);
    memcpy(nslots + slot, g->slots + slot + 1,
           sizeof(void *) * (GROUP_SIZE - slot));
    free(g->slots);
    g->slots = nslots;
    g->fill--;
    bmclear(g->mask, i);
    return 0;
}

int
spgroup_set(spgroup g, uint16_t i, void *v)
{
    uint16_t slot;

    if (!v) return spgroup_rm(g, i);

    if (!bmtest(g->mask, i)) {
        void **nslots;

        nslots = realloc(g->slots, sizeof(void *) * (g->fill + 1));
        if (!nslots) return warn("realloc"), -1;

        g->slots = nslots;
        g->fill++;
    }

    slot = bmcount(g->mask, i);
    g->slots[slot] = v;
    bmset(g->mask, i);
    return 0;
}

/* Sparse Array */

sparr
make_sparr(int cap)
{
    sparr a;
    int ngroups = cap / GROUP_SIZE;

    a = malloc(sizeof(struct sparr) + ngroups * sizeof(spgroup));
    if (!a) return warn("malloc"), (void *) 0;

    a->cap = cap;
    a->fill = 0;
    memset(a->groups, 0, ngroups * sizeof(spgroup));

    return a;
}

static size_t
group_num(size_t i)
{
    return i / GROUP_SIZE;
}

static uint16_t
pos_in_group(size_t i)
{
    return i % GROUP_SIZE;
}

void *
sparr_get(sparr a, size_t i)
{
    size_t gnum = group_num(i);

    if (!a->groups[gnum]) a->groups[gnum] = make_spgroup();
    if (!a->groups[gnum]) return warnx("make_spgroup"), (void *) 0;

    return spgroup_get(a->groups[gnum], pos_in_group(i));
}

int
sparr_set(sparr a, size_t i, void *v)
{
    int r;
    uint16_t old_g_fill;
    spgroup g;
    size_t gnum = group_num(i);

    if (!a->groups[gnum]) a->groups[gnum] = make_spgroup();
    if (!a->groups[gnum]) return warnx("make_spgroup"), -1;

    g = a->groups[gnum];

    old_g_fill = g->fill;
    r = spgroup_set(g, pos_in_group(i), v);
    a->fill += g->fill - old_g_fill;
    return r;
}

int
sparr_rm(sparr a, size_t i)
{
    return sparr_set(a, i, 0);
}

