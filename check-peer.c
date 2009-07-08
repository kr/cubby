#include <stdlib.h>
#include <stdio.h>

#include "cut.h"
#include "peer.h"

void
__CUT_BRINGUP__void_peer()
{
}

void
__CUT__void_peer_test_make_peer()
{
    peer p = make_peer(0, 0, 0);
    ASSERT(p, "");
}

void
__CUT_TAKEDOWN__void_peer()
{
}

