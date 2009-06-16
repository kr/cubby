#include <stdlib.h>
#include <stdio.h>

#include "cut.h"
#include "peer.h"

void
__CUT_BRINGUP__void_peer()
{
}

void
__CUT__void_peer_test_make_peer_no_key()
{
    peer p = make_peer(0);
    ASSERT(!p, "");
}

void
__CUT__void_peer_test_make_peer()
{
    uint32_t key[3] = { 1, 2, 3 };

    peer p = make_peer(key);
    ASSERT(p, "");
    ASSERT(p->key[0] == key[0], "");
    ASSERT(p->key[1] == key[1], "");
    ASSERT(p->key[2] == key[2], "");
}

void
__CUT_TAKEDOWN__void_peer()
{
}

