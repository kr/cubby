#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "cut.h"
#include "peer.h"

void
__CUT_BRINGUP__void_peer()
{
}

void
__CUT__void_peer_test_make_peer()
{
    uint32_t exp[3] = { 0x81fb346d, 0x17368bd9, 0xd5dd941e };

    peer p = make_peer(0, inet_addr("127.0.0.1"), htons(20202));
    ASSERT(p, "");
    ASSERT(p->key[0] == exp[0], "expected 0x%x, got 0x%x", exp[0], p->key[0]);
    ASSERT(p->key[1] == exp[1], "expected 0x%x, got 0x%x", exp[1], p->key[1]);
    ASSERT(p->key[2] == exp[2], "expected 0x%x, got 0x%x", exp[2], p->key[2]);
}

void
__CUT_TAKEDOWN__void_peer()
{
}

