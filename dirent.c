// dirent.c -- Directory Entry

#include <stdlib.h>
#include <string.h>

#include "dirent.h"
#include "region.h"
#include "util.h"

static struct dirent invalid_dirent_s = {};
dirent invalid_dirent = &invalid_dirent_s;

dirent
make_dirent(uint32_t *key, uint8_t len)
{
    dirent d;

    d = malloc(sizeof(struct dirent) + sizeof(struct rdesc) * len);
    if (!d) return warn("malloc"), (dirent) 0;

    d->len = len;
    d->key[0] = key[0];
    d->key[1] = key[1];
    d->key[2] = key[2];
    memset(d->rdescs, 0, sizeof(struct rdesc) * len);

    return d;
}

int
dirent_matches(dirent d, uint32_t *key)
{
    if (!d) return 0;
    return d->key[0] == key[0] && d->key[1] == key[1] && d->key[2] == key[2];
}

void
dirent_set_rdesc_local(dirent d, int i, region r, blob b)
{
    rdesc_local desc = (rdesc_local) &d->rdescs[i];

    desc->flags |= RDESC_LOCAL;
    //desc->flags &= ~RDESC_REMOTE;
    desc->reg = r->id;
    desc->off = region_blob_offset(r, b);
}
