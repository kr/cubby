// prot.h -- Protocol header

#ifndef prot_h
#define prot_h

#include "manager.h"

typedef void(*prot_send_link_fn)(manager, uint32_t *, int, void *);

void prot_init();

/* Do any work that remains to be done before waiting on the network. */
void prot_work(manager mgr);

int prot_outstanding_link_onremove(arr a, void *item, size_t index);
void prot_send_links(manager m, uint32_t *k, prot_send_link_fn cb, void *data);

#endif //prot_h
