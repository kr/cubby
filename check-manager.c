#include <stdlib.h>
#include <unistd.h>

#include "manager.h"
#include "cut.h"

static struct manager mgr = {};
static struct region r1, r2, r3;
static char s1[1024], s2[2048], s3[2048];

void
__CUT_BRINGUP__manager()
{
    heap_init(&mgr.region_pool, (cmp_fn) region_space_cmp);
    region_init(&r1, 1, (region_storage) s1, sizeof(s1));
    region_init(&r2, 2, (region_storage) s2, sizeof(s2));
    region_init(&r3, 3, (region_storage) s3, sizeof(s3));
}

void
__CUT__manager_with_no_free_regions()
{
    ASSERT(!mgr.region_pool.used, "should be empty");
}

void
__CUT__manager_add_one_free_region()
{
    manager_add_free_region(&mgr, &r1);
    ASSERT(heap_peek(&mgr.region_pool) == &r1, "should be just r1");
}

void
__CUT__manager_add_two_free_regions()
{
    manager_add_free_region(&mgr, &r1);
    manager_add_free_region(&mgr, &r2);

    ASSERT(heap_take(&mgr.region_pool) == &r1, "should be r1, r2");
    ASSERT(heap_take(&mgr.region_pool) == &r2, "should be r1, r2");
}

void
__CUT__manager_add_two_free_regions_other_order()
{
    manager_add_free_region(&mgr, &r2);
    manager_add_free_region(&mgr, &r1);

    ASSERT(heap_take(&mgr.region_pool) == &r1, "should be r1, r2");
    ASSERT(heap_take(&mgr.region_pool) == &r2, "should be r1, r2");
}

void
__CUT__manager_add_two_equal_free_regions()
{
    manager_add_free_region(&mgr, &r2);
    manager_add_free_region(&mgr, &r3);

    ASSERT(heap_take(&mgr.region_pool) == &r2, "should be r2, r3");
    ASSERT(heap_take(&mgr.region_pool) == &r3, "should be r2, r3");
}

void
__CUT__manager_add_two_equal_free_regions_other_order()
{
    manager_add_free_region(&mgr, &r3);
    manager_add_free_region(&mgr, &r2);

    ASSERT(heap_take(&mgr.region_pool) == &r2, "should be r2, r3");
    ASSERT(heap_take(&mgr.region_pool) == &r3, "should be r2, r3");
}

void
__CUT_TAKEDOWN__manager()
{
}

void
__CUT_BRINGUP__manager_with_regions()
{
    heap_init(&mgr.region_pool, (cmp_fn) region_space_cmp);
    region_init(&r1, 1, (region_storage) s1, sizeof(s1));
    region_init(&r2, 2, (region_storage) s2, sizeof(s2));
    region_init(&r3, 3, (region_storage) s3, sizeof(s3));
    manager_add_free_region(&mgr, &r1);
    manager_add_free_region(&mgr, &r2);
}

void
__CUT__manager_with_regions_pick_first()
{
    region r;

    r = manager_pick_region(&mgr, 500);
    ASSERT(r == &r1, "");

    ASSERT(heap_take(&mgr.region_pool) == &r2, "should be just r2");
}

void
__CUT__manager_with_regions_pick_second()
{
    region r;

    r = manager_pick_region(&mgr, 1500);
    ASSERT(r == &r2, "");

    ASSERT(heap_take(&mgr.region_pool) == &r1, "should be just r1");
}

void
__CUT__manager_with_regions_pick_large_then_small()
{
    region r;

    r = manager_pick_region(&mgr, 2500);
    ASSERT(!r, "");

    ASSERT(mgr.region_pool.heap[0] == &r1, "should be r1, r2");
    ASSERT(mgr.region_pool.heap[1] == &r2, "should be r1, r2");

    r = manager_pick_region(&mgr, 500);
    ASSERT(r == &r1, "");

    ASSERT(mgr.region_pool.heap[0] == &r2, "should be r1, r2");
}

void
__CUT__manager_with_regions_pick_lots()
{
    region r;

    r = manager_pick_region(&mgr, 50);
    ASSERT(r == &r1, "");
    heap_give(&mgr.region_pool, r);
    r = manager_pick_region(&mgr, 50);
    ASSERT(r == &r1, "");
    heap_give(&mgr.region_pool, r);
    r = manager_pick_region(&mgr, 50);
    ASSERT(r == &r1, "");
    heap_give(&mgr.region_pool, r);
    r = manager_pick_region(&mgr, 50);
    ASSERT(r == &r1, "");
    heap_give(&mgr.region_pool, r);
    r = manager_pick_region(&mgr, 50);
    ASSERT(r == &r1, "");
    heap_give(&mgr.region_pool, r);
    r = manager_pick_region(&mgr, 50);
    ASSERT(r == &r1, "");
    heap_give(&mgr.region_pool, r);
    r = manager_pick_region(&mgr, 50);
    ASSERT(r == &r1, "");
    heap_give(&mgr.region_pool, r);
    r = manager_pick_region(&mgr, 50);
    ASSERT(r == &r1, "");
    heap_give(&mgr.region_pool, r);
    r = manager_pick_region(&mgr, 50);
    ASSERT(r == &r1, "");
    heap_give(&mgr.region_pool, r);
    r = manager_pick_region(&mgr, 50);
    ASSERT(r == &r1, "");
    heap_give(&mgr.region_pool, r);
    r = manager_pick_region(&mgr, 50);
    ASSERT(r == &r1, "");
}

void
__CUT__manager_with_regions_allocate_lots()
{
    blob b;
    uint32_t key[3] = {};
    dirent de;

    de = make_dirent(key, 1, 0);

    b = manager_allocate_blob(&mgr, de, 50);
    ASSERT(b, "");
    b = manager_allocate_blob(&mgr, de, 50);
    ASSERT(b, "");
    b = manager_allocate_blob(&mgr, de, 50);
    ASSERT(b, "");
    b = manager_allocate_blob(&mgr, de, 50);
    ASSERT(b, "");
    b = manager_allocate_blob(&mgr, de, 50);
    ASSERT(b, "");
    b = manager_allocate_blob(&mgr, de, 50);
    ASSERT(b, "");
    b = manager_allocate_blob(&mgr, de, 50);
    ASSERT(b, "");
    b = manager_allocate_blob(&mgr, de, 50);
    ASSERT(b, "");
    b = manager_allocate_blob(&mgr, de, 50);
    ASSERT(b, "");
    b = manager_allocate_blob(&mgr, de, 50);
    ASSERT(b, "");
    b = manager_allocate_blob(&mgr, de, 50);
    ASSERT(b, "");
}

void
__CUT_TAKEDOWN__manager_with_regions()
{
}


static int tmp_bundle_size = 10485760;
static char tmp_bundle_name_tpl[] = "/tmp/XXXXXX",
            tmp_bundle_name[sizeof(tmp_bundle_name_tpl) + 1];

void
__CUT_BRINGUP__manager_with_init()
{
    int r;

    initialize_bundles = 1;

    strcpy(tmp_bundle_name, tmp_bundle_name_tpl);
    int tmp_bundle = mkstemp(tmp_bundle_name);

    r = lseek(tmp_bundle, tmp_bundle_size - 1, SEEK_SET);
    ASSERT(r == tmp_bundle_size - 1, "error");
    r = write(tmp_bundle, "\0", 1);
    ASSERT(r == 1, "error");
    r = lseek(tmp_bundle, 0, SEEK_SET);
    ASSERT(r == 0, "error");

    close(tmp_bundle);

    mgr = (struct manager) {};
    add_bundle(&mgr, tmp_bundle_name);

    r = manager_init(&mgr);
    ASSERT(r == 0, "error");
}

void
__CUT__manager_with_init_check_key_chain_len()
{
    ASSERT(mgr.key_chain_len > 0, "should be positive");
}

void
__CUT__manager_with_init_get_peer()
{
    struct in_addr host_addr = {};
    int udp_port = 0;
    key_for_peer(mgr.key, host_addr.s_addr, udp_port);
    manager_get_peer(&mgr, host_addr.s_addr, udp_port);
    ASSERT(mgr.peers_fill == 1, "should be one peer");
}

void
__CUT_TAKEDOWN__manager_with_init()
{
    unlink(tmp_bundle_name);
}

void
__CUT_BRINGUP__manager_with_key()
{
    int r;

    initialize_bundles = 1;

    strcpy(tmp_bundle_name, tmp_bundle_name_tpl);
    int tmp_bundle = mkstemp(tmp_bundle_name);

    r = lseek(tmp_bundle, tmp_bundle_size - 1, SEEK_SET);
    ASSERT(r == tmp_bundle_size - 1, "error");
    r = write(tmp_bundle, "\0", 1);
    ASSERT(r == 1, "error");
    r = lseek(tmp_bundle, 0, SEEK_SET);
    ASSERT(r == 0, "error");

    close(tmp_bundle);

    mgr = (struct manager) {};
    add_bundle(&mgr, tmp_bundle_name);

    r = manager_init(&mgr);
    ASSERT(r == 0, "error");

    struct in_addr host_addr = {};
    int udp_port = 0;
    key_for_peer(mgr.key, host_addr.s_addr, udp_port);
    mgr.self = manager_get_peer(&mgr, host_addr.s_addr, udp_port);
    mgr.self->is_local = 1;
}

void
__CUT__manager_with_key_merge_node()
{
    int r = manager_merge_node(&mgr, mgr.key, mgr.self);
    ASSERT(r == 0, "should succeed");
    ASSERT(mgr.nodes.used == 1, "should be 1");
}

void
__CUT__manager_with_key_merge_duplicate_node()
{
    int r;
    r = manager_merge_node(&mgr, mgr.key, mgr.self);
    ASSERT(r == 0, "should succeed");
    r = manager_merge_node(&mgr, mgr.key, mgr.self);
    ASSERT(r == 0, "should succeed");
    ASSERT(mgr.nodes.used == 1, "should still be 1");
}

void
__CUT__manager_with_key_merge_nodes()
{
    manager_merge_nodes(&mgr, mgr.key_chain_len, mgr.key, mgr.self);
    ASSERT(mgr.nodes.used == 1, "should be 1");
}

void
__CUT__manager_with_key_merge_duplicate_nodes()
{
    manager_merge_nodes(&mgr, mgr.key_chain_len, mgr.key, mgr.self);
    manager_merge_nodes(&mgr, mgr.key_chain_len, mgr.key, mgr.self);
    ASSERT(mgr.nodes.used == 1, "should still be 1");
}

void
__CUT__manager_with_key_merge_two_nodes()
{
    struct in_addr host_addr = {};
    int udp_port = 1;
    uint32_t key[3] = {};
    peer p;

    key_for_peer(key, host_addr.s_addr, udp_port);
    p = manager_get_peer(&mgr, host_addr.s_addr, udp_port);

    manager_merge_nodes(&mgr, mgr.key_chain_len, mgr.key, mgr.self);
    manager_merge_nodes(&mgr, 1, key, p);
    ASSERT(mgr.nodes.used == 2, "should have 2 nodes, got %d", mgr.nodes.used);
}

void
__CUT__manager_with_key_find_owners_self()
{
    struct in_addr host_addr = {};
    int udp_port = 1;
    uint32_t other_key[3] = {};
    peer other;

    key_for_peer(other_key, host_addr.s_addr, udp_port);
    other = manager_get_peer(&mgr, host_addr.s_addr, udp_port);

    manager_merge_nodes(&mgr, mgr.key_chain_len, mgr.key, mgr.self);
    manager_merge_nodes(&mgr, 1, other_key, other);

    uint32_t obj_key[3];
    node nodes[2];
    obj_key[0] = mgr.key[0];
    obj_key[1] = mgr.key[1];
    obj_key[2] = mgr.key[2];
    obj_key[2]++;
    int n = manager_find_owners(&mgr, obj_key, 2, nodes);
    ASSERT(n == 2, "should have found 2 nodes");
    ASSERT(nodes[0]->peer == mgr.self, "first peer should be self");
    ASSERT(nodes[1]->peer == other, "second peer should be other");
}

void
__CUT__manager_with_key_find_owners_other()
{
    struct in_addr host_addr = {};
    int udp_port = 1;
    uint32_t other_key[3] = {};
    peer other;

    key_for_peer(other_key, host_addr.s_addr, udp_port);
    other = manager_get_peer(&mgr, host_addr.s_addr, udp_port);

    manager_merge_nodes(&mgr, mgr.key_chain_len, mgr.key, mgr.self);
    manager_merge_nodes(&mgr, 1, other_key, other);

    uint32_t obj_key[3];
    node nodes[2];
    obj_key[0] = other_key[0];
    obj_key[1] = other_key[1];
    obj_key[2] = other_key[2];
    obj_key[2]++;
    int n = manager_find_owners(&mgr, obj_key, 2, nodes);
    ASSERT(n == 2, "should have found 2 nodes");
    ASSERT(nodes[0]->peer == other, "first peer should be other");
    ASSERT(nodes[1]->peer == mgr.self, "second peer should be self");
}

void
__CUT__manager_with_key_add_no_link()
{
    uint32_t key[3] = {};

    manager_merge_nodes(&mgr, mgr.key_chain_len, mgr.key, mgr.self);

    dirent de = manager_add_links(&mgr, key, 0, 0, 0);
    //ASSERT(de == 0, "should have no dirent");
    ASSERT(de->len == 0, "should have an empty dirent");
}

void
__CUT__manager_with_key_add_link()
{
    uint32_t key[3] = {};
    peer_id id = 12345;

    manager_merge_nodes(&mgr, mgr.key_chain_len, mgr.key, mgr.self);

    dirent de = manager_add_links(&mgr, key, 0, 1, &id);
    ASSERT(de, "should have a dirent");
    ASSERT(de->len == 1, "should have a dirent with an entry");
}

void
__CUT__manager_with_key_add_self_link()
{
    uint32_t key[3] = {};
    peer_id id = make_peer_id(mgr.self->addr, mgr.self->cp_port);

    manager_merge_nodes(&mgr, mgr.key_chain_len, mgr.key, mgr.self);

    dirent de = manager_add_links(&mgr, key, 0, 1, &id);
    ASSERT(de, "should have a dirent");
    ASSERT(de->len == 0, "should have NO self-linking entry, got %d", de->len);
}

void
__CUT_TAKEDOWN__manager_with_key()
{
    unlink(tmp_bundle_name);
}

