#include <string.h>

#include "bundle.h"
#include "manager.h"
#include "prot.h"
#include "key.h"
#include "util.h"

void
manager_add_free_region(manager m, region r)
{
    heap_give(&m->region_pool, r);
}

/* This will REMOVE a region from the free pool. The caller is responsible for
   putting it back. */
region
manager_pick_region(manager m, size_t size)
{
    region r, tmp = 0;

    // Pull out regions until we find one that's got enough space.
    r = heap_take(&m->region_pool);
    while (r && !region_has_space_for_blob(r, size)) {
        r->next = tmp;
        tmp = r;
        r = heap_take(&m->region_pool);
    }

    // Put back the ones we didn't use.
    for (; tmp; tmp = tmp->next) heap_give(&m->region_pool, tmp);

    return r;
}

blob
manager_allocate_blob(manager m, dirent d, size_t size)
{
    region r;
    blob b;

    r = manager_pick_region(m, size);
    if (!r) return 0;

    raw_warnx("allocating blob in region %d\n", r->id);

    b = region_allocate_blob(r, size);
    if (!b) return 0;

    dirent_set_rdesc_local(d, 0, r, b);

    /* Put r back in the free pool. */
    heap_give(&m->region_pool, r);

    return b;
}

region
manager_get_region(manager m, dirent d)
{
    rdesc_local rd;

    rd = dirent_get_rdesc_local(d);
    return &m->all_regions[rd->reg];
}

blob
manager_get_blob(manager m, dirent d)
{
    rdesc_local rd;

    rd = dirent_get_rdesc_local(d);
    return region_get_blob_by_off(manager_get_region(m, d), rd->off);
}

void
manager_delete_blob(manager m, dirent d)
{
    blob b;
    region r;

    r = manager_get_region(m, d);
    b = manager_get_blob(m, d);
    region_delete_blob(r, b);
}

static int
region_close_to_full(region r)
{
    return 0;
}

static int
manager_read_regions(manager mgr, uint16_t count)
{
    int i, j, n = 0;

    mgr->all_regions = malloc(sizeof(struct region) * count);
    if (!mgr->all_regions) return warn("malloc"), -1;
    mgr->nregions = count;

    for (i = 0; i < mgr->nbundles; i++) {
        bundle bun = bundle_get(mgr, i);
        for (j = 0; j < bun->nregions; j++) {
            blob bl;
            size_t size;
            region reg = &mgr->all_regions[n++];

            // Total size of the region, including the headers
            size = 1LU << REGION_BITS;
            if (j == bun->nregions - 1) size = bun->reg_size % size;
            // the last region might be shorter

            region_init(reg, n - 1, bundle_get_region_storage(bun, j), size);

            for (bl = (blob) reg->storage->blobs; ; bl = blob_next(bl)) {
                int r;
                dirent de;
                rdesc_local rdesc;

                if (!bl->size) break;
                raw_warnx("blob size is %d", bl->size);

                /* This blob claims to extend past the end of the region! */
                if (((char *) bl) >= reg->top) {
                    warnx("%s: last blob overextended", bun->name);
                    break;
                }

                r = blob_verify(bl);
                if (r == -1) {
                    warnx("verification failed for blob at offset 0x%x\n",
                          region_blob_offset(reg, bl));
                    continue;
                }

                de = make_dirent(bl->key, 1);
                rdesc = (rdesc_local) &de->rdescs[0];
                rdesc->flags = RDESC_LOCAL;
                rdesc->reg = reg->id;
                rdesc->off = region_blob_offset(reg, bl);
                spht_set(mgr->directory, de);
                char fkey[27];
                key_fmt(fkey, bl->key);
                raw_warnx("read one k = %s", fkey);
            }
            reg->free = (char *) bl;
            if (!region_close_to_full(reg)) {
                manager_add_free_region(mgr, reg);
            }
        }
    }

    return 0;
}

int
manager_init(manager m)
{
    int i, r;
    size_t nregions = 0;

    // Initialize the heap if necessary
    heap_init(&m->region_pool, (cmp_fn) region_space_cmp);

    arr_init(&m->outstanding_links, 0, prot_outstanding_link_onremove);

    m->directory = make_spht(0);
    if (!m->directory) return warnx("making directory"), -1;

    if (m->nbundles < 1) return raw_warnx("no bundles defined"), -1;

    for (i = 0; i < m->nbundles; i++) {
        bundle b = m->all_bundles + i;
        r = bundle_open(b);
        if (r == -1) {
            warnx("error with bundle %s; skipping", b->name);
            continue;
        }
        nregions += b->nregions;
    }

    if (nregions < 1) return warnx("no valid regions"), -2;

    r = manager_read_regions(m, nregions);
    if (r == -1) return warnx("manager_read_regions"), -2;

    return 0;
}

static void
manager_grow_peers(manager m)
{
    size_t new_cap = (m->peers_cap + 1) << 1;
    peer *new = malloc(new_cap * sizeof(peer));

    if (!new) return warn("malloc");

    memcpy(new, m->peers, m->peers_fill * sizeof(peer));
    free(m->peers);
    m->peers = new;
    m->peers_cap = new_cap;
}

int
manager_insert_peer(manager m, peer p)
{

    if (m->peers_fill >= m->peers_cap) manager_grow_peers(m);
    if (m->peers_fill >= m->peers_cap) return -1;

    m->peers[m->peers_fill++] = p;
    return 0;
}

#include <stdio.h>

void
manager_out_add(manager m, cpkt c)
{
    c->next = 0;
    if (!m->out_head) {
        m->out_head = c;
    } else {
        m->out_tail->next = c;
    }
    m->out_tail = c;
}

int
manager_out_any(manager m)
{
    return !!m->out_head;
}

cpkt
manager_out_remove(manager m)
{
    cpkt c = m->out_head;

    if (!c) return 0;

    m->out_head = c->next;
    if (!m->out_head) m->out_tail = 0;

    return c;
}

void
manager_out_pushback(manager m, cpkt c)
{
    if (!m->out_head) m->out_tail = c;
    c->next = m->out_head;
    m->out_head = c;
}

/* If the peer does not exist, it will be created. */
peer
manager_get_peer(manager m, in_addr_t addr, uint16_t port)
{
    for (int i = 0; i < m->peers_fill; i++) {
        peer p = m->peers[i];
        if (p->addr == addr && p->cp_port == port) return p;
    }

    peer p = make_peer(m, addr, port);
    if (!p) return warnx("make_peer"), (peer) 0;

    int r = manager_insert_peer(m, p);
    if (r == -1) return warnx("manager_insert_peer"), (peer) 0;

    return p;
}

int
manager_find_closest_active_peers(manager m, uint32_t *key, int n, peer *out)
{
    peer ps[n + 1];

    int found = 0;
    for (int i = 0; i < m->peers_fill; i++) {
        if (!peer_active(m->peers[i])) continue;
        int j = min(found, n);
        found++;
        ps[j] = m->peers[i];
        for (; j; j--) {
            if (key_distance_cmp(m->key, ps[j - 1]->key, ps[j]->key) < 0) break;
            peer t = ps[j];
            ps[j] = ps[j - 1];
            ps[j - 1] = t;
        }
    }

    for (int i = 0; i < n; i++) out[i] = ps[i];
    return found;
}

int
manager_add_link(manager m, uint32_t *key, peer p)
{
    int r;
    dirent nde, de = spht_get(m->directory, key);

    if (de) {
        if (dirent_has_remote(de, p->addr, p->cp_port)) return 0;
        nde = copy_dirent(de, de->len + 1);
    } else {
        nde = make_dirent(key, 1);
    }
    if (!nde) return -1;

    r = dirent_add_remote(nde, p->addr, p->cp_port);
    if (r == -1) return free(nde), -1; // can't happen

    r = spht_set(m->directory, nde);
    if (r == -1) return free(nde), -1;

    free(de);

    return 0;
}
