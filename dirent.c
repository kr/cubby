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

dirent
copy_dirent(dirent d, uint8_t new_len)
{
    dirent nd;

    nd = malloc(sizeof(struct dirent) + sizeof(struct rdesc) * new_len);
    if (!nd) return warn("malloc"), (dirent) 0;

    nd->len = new_len;
    nd->key[0] = d->key[0];
    nd->key[1] = d->key[1];
    nd->key[2] = d->key[2];
    int common = min(d->len, new_len);
    for (int i = 0; i < common; i++) {
        nd->rdescs[i] = d->rdescs[i];
    }
    memset(nd->rdescs + common, 0, sizeof(struct rdesc) * (new_len - common));

    return nd;
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

rdesc_local
dirent_get_rdesc_local(dirent d)
{
    int i;

    for (i = 0; i < d->len; i++) {
        if (d->rdescs[i].flags & RDESC_LOCAL) break;
    }
    if (i >= d->len) return 0;

    return (rdesc_local) &d->rdescs[i];
}

int
dirent_has_remote(dirent d, in_addr_t addr, uint16_t port)
{
    for (int i = 0; i < d->len; i++) {
        rdesc rd = d->rdescs + i;
        if (rd->flags & RDESC_REMOTE &&
                rd->b == addr && rd->a == port) return 1;
    }
    return 0;
}

int
dirent_add_remote(dirent d, in_addr_t addr, uint16_t port)
{
    for (int i = 0; i < d->len; i++) {
        rdesc rd = d->rdescs + i;
        if (!rd->flags) {
            rd->flags |= RDESC_REMOTE;
            rd->a = port;
            rd->b = addr;
            return 0;
        }
    }
    return -1;
}

