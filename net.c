// net.c -- generic networking

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/http.h>

#include "http.h"
#include "manager.h"
#include "net.h"
#include "util.h"

static int memcache_socket = -1;

int
net_loop(struct event_base *ev_base)
{
    int r;

    r = event_base_dispatch(ev_base);
    // what is the meaning of this return value???
    // does event_base_dispatch set errno???
    return warnx("event_base_dispatch error %d", r), -1;
}

int
make_listen_socket(struct in_addr host_addr, int port)
{
    int fd, flags, r;
    struct linger linger = {0, 0};
    struct sockaddr_in saddr = {};

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) return warn("socket"), -1;

    // get existing flags
    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return warn("getting flags"), close(fd), -1;

    // set the all-important O_NONBLOCK
    r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (r == -1) return warn("setting O_NONBLOCK"), close(fd), -1;

    flags = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof flags);
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &flags, sizeof flags);
    setsockopt(fd, SOL_SOCKET, SO_LINGER,   &linger, sizeof linger);
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flags, sizeof flags);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr = host_addr;
    r = bind(fd, (const struct sockaddr *) &saddr, sizeof(saddr));
    if (r == -1) return warn("bind"), close(fd), -1;

    r = listen(fd, 1024); // 1024 == size of backlog of pending connections
    if (r == -1) return warn("listen"), close(fd), -1;

    return fd;
}

void
net_init(struct event_base *ev_base, struct in_addr host_addr,
         int memcache_port, int http_port, manager mgr)
{
    int r;
    struct evhttp *ev_http;

    if (memcache_port) {
        memcache_socket = make_listen_socket(host_addr, memcache_port);
        if (memcache_socket == -1) {
            warnx("could not open memcache port %d", memcache_port);
            exit(2);
        }
    }

    if (http_port) {
        ev_http = evhttp_new(ev_base);
        r = evhttp_bind_socket(ev_http, "0.0.0.0", http_port);
        // This return value is undocumented (!), but this seems to work.
        if (r == -1) {
            warn("evhttp_bind_socket");
            warnx("could not open port %d", http_port);
            exit(3);
        }

        evhttp_set_gencb(ev_http, http_handle_generic, mgr);
        evhttp_set_cb(ev_http, "/", http_handle_root, mgr);
    }

}
