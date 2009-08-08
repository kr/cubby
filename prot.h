// prot.h -- Protocol header

#ifndef prot_h
#define prot_h

#include "manager.h"

#define DIRENT_W 2

typedef void(*prot_send_link_fn)(manager, uint32_t *, int, void *);

void prot_init();

/* Do any work that remains to be done before waiting on the network. */
void prot_work(manager mgr);

int prot_outstanding_link_onremove(arr a, void *item, size_t index);

void prot_send_links(manager m, int n, peer *to, dirent de, uint8_t rank,
        prot_send_link_fn cb, void *data);
void prot_send_link(manager m, dirent de, prot_send_link_fn cb, void *data);
void prot_linked(peer p, uint32_t *key);

#endif //prot_h
