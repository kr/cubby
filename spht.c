// spht.c -- Slow but memory-efficient sparse hashtable.

/*

This is exactly the same idea as what you'll find in Google's sparsehash
package, but without the STL and C++ diarrhea.

http://code.google.com/p/google-sparsehash/

*/

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "dirent.h"
#include "spht.h"
#include "util.h"

#define INVALID_BUCKET ((size_t) ~0)

#define MIN_CAP 4

typedef struct position {
    size_t exists, insert;
} *position;

spht
make_spht(size_t cap_wanted)
{
    spht h;
    size_t cap;

    for (cap = MIN_CAP; cap < cap_wanted; cap <<= 1);

    h = malloc(sizeof(struct spht));
    if (!h) return warn("malloc"), (spht) 0;

    h->ndel = 0;
    h->table = make_sparr(cap);
    if (!h->table) return warn("malloc"), free(h), (spht) 0;

    return h;
}

static int
resize(spht h, size_t delta)
{
    return 0;
}

static int
test_deleted(spht h, size_t pos)
{
    return sparr_get(h->table, pos) == INVALID_DIRENT;
}

static void
find_position(spht h, position pos, key k)
{
    size_t num_probes = 0;
    const size_t mask = h->table->cap - 1;
    size_t bucket = k->a & mask;
    pos->insert = INVALID_BUCKET;

    for (;;) {
        if (!sparr_test(h->table, bucket)) {
            pos->exists = INVALID_BUCKET;

            // Is this the first empty spot?
            if (pos->insert == INVALID_BUCKET) {
                pos->insert = bucket;
            }

            return;
        } else if (test_deleted(h, bucket)) {
            if (pos->insert == INVALID_BUCKET) pos->insert = bucket;
        } else if (dirent_matches(sparr_get(h->table, bucket), k)) {
            pos->exists = bucket;
            pos->insert = INVALID_BUCKET;
            return;
        }
        ++num_probes;
        if (num_probes > 100) return;
        bucket = (bucket + num_probes) & mask;
    }


    pos->exists = INVALID_BUCKET;
    pos->insert = 0;
}

dirent
spht_get(spht h, key k)
{
    struct position pos;

    find_position(h, &pos, k);

    if (pos.exists == INVALID_BUCKET) return 0;
    return sparr_get(h->table, pos.exists);
}

int
spht_set(spht h, key k, dirent v)
{
    int r;
    struct position pos;

    if (!v || v == INVALID_DIRENT) return spht_rm(h, k), 0;

    r = resize(h, 1);
    if (r == -1) return warnx("resize"), -1;

    find_position(h, &pos, k);

    // Already there.
    if (pos.exists != INVALID_BUCKET) return 0;

    if (test_deleted(h, pos.insert)) --h->ndel;

    sparr_set(h->table, pos.insert, v);
    return 0;
}

void
spht_rm(spht h, key k)
{
    struct position pos;

    find_position(h, &pos, k);

    if (pos.exists == INVALID_BUCKET) return;

    ++h->ndel;
    sparr_set(h->table, pos.exists, INVALID_DIRENT);
}

size_t
spht_fill(spht h)
{
    return h->table->fill - h->ndel;
}

