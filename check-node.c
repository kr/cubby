#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "cut.h"
#include "node.h"

void
__CUT_BRINGUP__void_node()
{
}

void
__CUT__void_node_test_make_node_local()
{
    uint32_t exp[3] = { 0x81fb346d, 0x17368bd9, 0xd5dd941e };
    bundle b = (bundle) 5;

    node n = make_node_local(exp, b);
    ASSERT(n, "");
    ASSERT(n->key[0] == exp[0], "expected 0x%x, got 0x%x", exp[0], n->key[0]);
    ASSERT(n->key[1] == exp[1], "expected 0x%x, got 0x%x", exp[1], n->key[1]);
    ASSERT(n->key[2] == exp[2], "expected 0x%x, got 0x%x", exp[2], n->key[2]);
    ASSERT(n->bundle == b, "expected %p, got %p", b, n->bundle);
    ASSERT(!n->peer, "expected %p, got %p", 0, n->peer);
}

void
__CUT__void_node_test_make_node_remote()
{
    uint32_t exp[3] = { 0x81fb346d, 0x17368bd9, 0xd5dd941e };
    peer p = (peer) 5;

    node n = make_node_remote(exp, p);
    ASSERT(n, "");
    ASSERT(n->key[0] == exp[0], "expected 0x%x, got 0x%x", exp[0], n->key[0]);
    ASSERT(n->key[1] == exp[1], "expected 0x%x, got 0x%x", exp[1], n->key[1]);
    ASSERT(n->key[2] == exp[2], "expected 0x%x, got 0x%x", exp[2], n->key[2]);
    ASSERT(n->peer == p, "expected %p, got %p", p, n->peer);
    ASSERT(!n->bundle, "expected %p, got %p", 0, n->bundle);
}

void
__CUT_TAKEDOWN__void_node()
{
}

