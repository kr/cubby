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
#define FILL_RATIO 0.8

typedef struct position {
    size_t exists, insert;
} *position;

static size_t
valid_size(size_t elements)
{
    size_t cap = MIN_CAP;

    while (elements >= cap * FILL_RATIO) cap <<= 1;
    return cap;
}

spht
make_spht(size_t elements)
{
    spht h;
    size_t cap;

    cap = valid_size(elements);

    h = malloc(sizeof(struct spht));
    if (!h) return warn("malloc"), (spht) 0;

    h->ndel = 0;
    h->table = make_sparr(cap);
    if (!h->table) return warn("malloc"), free(h), (spht) 0;
    h->enlarge_threshold = cap * FILL_RATIO;

    return h;
}

static int
test_deleted(sparr table, size_t pos)
{
    return sparr_get(table, pos) == invalid_dirent;
}

static void
find_position(sparr table, position pos, uint32_t *key)
{
    size_t num_probes = 0;
    const size_t mask = table->cap - 1;

    size_t bucket = key[0] & mask;

    pos->insert = INVALID_BUCKET;
    for (;;) {
        if (!sparr_test(table, bucket)) {
            pos->exists = INVALID_BUCKET;

            // Is this the first empty spot?
            if (pos->insert == INVALID_BUCKET) pos->insert = bucket;

            return;
        } else if (test_deleted(table, bucket)) {
            // Is this the first empty spot?
            if (pos->insert == INVALID_BUCKET) pos->insert = bucket;
        } else if (dirent_matches(sparr_get(table, bucket), key)) {
            pos->exists = bucket;
            pos->insert = INVALID_BUCKET;
            return;
        }
        ++num_probes;
        bucket = (bucket + num_probes) & mask;
    }

    pos->exists = INVALID_BUCKET;
    pos->insert = 0;
}

/* return 0 on success, -1 on failure */
static int
ensure_cap(spht h, size_t delta)
{
    int r;
    size_t i;
    sparr new_table;

    if (h->table->fill + delta <= h->enlarge_threshold) return 0;

    // Don't allocate space for deleted items. This means we might shrink if
    // there are a lot of deleted items, but that's okay.
    new_table = make_sparr(valid_size(h->table->fill + delta - h->ndel));
    if (!new_table) return warnx("make_sparr"), -1;

    // Jumping through all these hoops just to avoid some unnecessary checks
    // while rehashing. With good branch prediction it might be better just to
    // use the normal make_spht, spht_set, etc.
    for (i = 0; i < h->table->cap; i++) {
        struct position pos;
        dirent v = sparr_get(h->table, i);

        // Possibly could speed up this loop by skipping over empty buckets,
        // with assistance from sparr/spgroup.
        if (!v || v == invalid_dirent) continue;

        find_position(new_table, &pos, v->key);
        r = sparr_set(new_table, pos.insert, v);
        if (r == -1) {
            warnx("sparr_set");
            sparr_free(new_table);
            return -1;
        }
    }

    // Become the new table.
    sparr_free(h->table);
    h->ndel = 0;
    h->table = new_table;
    h->enlarge_threshold = h->table->cap * FILL_RATIO;

    return 0;
}

dirent
spht_get(spht h, uint32_t *k)
{
    struct position pos;

    find_position(h->table, &pos, k);

    if (pos.exists == INVALID_BUCKET) return 0;
    return sparr_get(h->table, pos.exists);
}

int
spht_set(spht h, dirent v)
{
    int r;
    struct position pos;

    if (!v || v == invalid_dirent) return -1;

    r = ensure_cap(h, 1);
    if (r == -1) return warnx("ensure_cap"), -1;

    find_position(h->table, &pos, v->key);

    // Already there?
    if (pos.exists != INVALID_BUCKET) {
        sparr_set(h->table, pos.exists, v);
    } else {
        if (test_deleted(h->table, pos.insert)) --h->ndel;
        sparr_set(h->table, pos.insert, v);
    }
    return 0;
}

void
spht_rm(spht h, uint32_t *k)
{
    struct position pos;

    find_position(h->table, &pos, k);

    if (pos.exists == INVALID_BUCKET) return;

    ++h->ndel;
    sparr_set(h->table, pos.exists, invalid_dirent);
}

size_t
spht_fill(spht h)
{
    return h->table->fill - h->ndel;
}

