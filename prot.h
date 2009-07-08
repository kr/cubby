// prot.h -- Protocol header

#ifndef prot_h
#define prot_h

void prot_init();

/* Do any work that remains to be done before waiting on the network. */
void prot_work(manager mgr);

#endif //prot_h
