// prot.c -- Protocol

#include "net.h"
#include "manager.h"

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
}
