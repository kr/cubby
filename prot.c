// prot.c -- Protocol

#include "net.h"
#include "manager.h"

static int my_id;

void
prot_work(manager mgr)
{
    for (int i = 0; i < mgr->peers_fill; i++) {
        peer_update(mgr->peers[i]);
    }
}


void
prot_init()
{
    my_id = 1;
}
