// dirent.c -- Directory Entry

#include <stdlib.h>

#include "dirent.h"
#include "util.h"

#define DIRENT_STORED_TYPE 0
#define DIRENT_REMOTE_TYPE 1

dirent
make_dirent_stored(key k)
{
    dirent d;
    dirent_stored ds;

    ds = malloc(sizeof(struct dirent_stored));
    if (!ds) return warn("malloc"), (dirent) 0;

    ds->key_hint = k->a;

    d = (dirent) ds;
    d->type = DIRENT_STORED_TYPE;

    return d;
}

dirent
make_dirent_remote(key k)
{
    dirent_remote dr;

    dr = malloc(sizeof(struct dirent_remote));
    if (!dr) return warn("malloc"), (dirent) 0;

    dr->type = DIRENT_REMOTE_TYPE;
    dr->k = *k;

    return (dirent) dr;
}

static int
dirent_stored_matches(dirent_stored ds, key k)
{
    return k->a == ds->key_hint;
    // TODO lookup the record and check the full key
}

static int
dirent_remote_matches(dirent_remote dr, key k)
{
    return k->a == dr->k.a && k->b == dr->k.b;
}

int
dirent_matches(dirent d, key k)
{
    if (!d || d == INVALID_DIRENT) return 0;

    if (d->type == DIRENT_STORED_TYPE) {
        return dirent_stored_matches((dirent_stored) d, k);
    }

    if (d->type == DIRENT_REMOTE_TYPE) {
        return dirent_remote_matches((dirent_remote) d, k);
    }

    return 0; // can't happen
}

