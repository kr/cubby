// net.h -- generic networking header

#ifndef net_h
#define net_h

#include <netinet/in.h>
#include <event2/event.h>

#include "manager.h"

/* Initialize the network. */
void net_init(struct in_addr host_addr, int udp_port, manager mgr);

/* The main dispatch loop. */
int net_loop(manager mgr);

/* Check if we have outbound data and set an event if necessary. */
void net_update_ev(manager mgr);

#endif //net_h
