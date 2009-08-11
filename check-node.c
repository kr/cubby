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
__CUT__void_node_test_make_node_null_key()
{
    peer p = make_peer(0, 0, 0);
    node n = make_node(0, p);
    ASSERT(!n, "");
}

void
__CUT__void_node_test_make_node_null_peer()
{
    uint32_t exp[3] = { 0x81fb346d, 0x17368bd9, 0xd5dd941e };

    node n = make_node(exp, 0);
    ASSERT(!n, "");
}

void
__CUT__void_node_test_make_node()
{
    uint32_t exp[3] = { 0x81fb346d, 0x17368bd9, 0xd5dd941e };
    peer p = make_peer(0, 0, 0);

    node n = make_node(exp, p);
    ASSERT(n, "");
    ASSERT(n->key[0] == exp[0], "expected 0x%x, got 0x%x", exp[0], n->key[0]);
    ASSERT(n->key[1] == exp[1], "expected 0x%x, got 0x%x", exp[1], n->key[1]);
    ASSERT(n->key[2] == exp[2], "expected 0x%x, got 0x%x", exp[2], n->key[2]);
    ASSERT(n->peer == p, "expected %p, got %p", p, n->peer);
}

void
__CUT_TAKEDOWN__void_node()
{
}

