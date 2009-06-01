// net.h -- generic networking header

#ifndef net_h
#define net_h

#include <netinet/in.h>
#include <event2/event.h>

/* Initialize the network. */
void net_init(struct event_base *ev_base, struct in_addr host_addr,
              int memcache_port, int http_port);

/* The main dispatch loop. */
int net_loop();

#endif //net_h
