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
#include <errno.h>
#include <event2/event.h>
#include <event2/http.h>

#include "http.h"
#include "manager.h"
#include "net.h"
#include "prot.h"
#include "util.h"

static struct event_base *ev_base;
static struct event *udp_ev;
static int udp_socket;
static int memcache_socket = -1;


int
net_loop(manager mgr)
{
    int r;

    net_update_ev(mgr);

    r = event_base_dispatch(ev_base);
    // what is the meaning of this return value???
    // does event_base_dispatch set errno???
    return warnx("event_base_dispatch error %d", r), -1;
}

int
make_udp_listen_socket(struct in_addr host_addr, int port)
{
    int fd, flags, r;
    struct linger linger = {0, 0};
    struct sockaddr_in saddr = {};

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) return warn("socket"), -1;

    // get existing flags
    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return warn("getting flags"), close(fd), -1;

    // set the all-important O_NONBLOCK
    r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (r == -1) return warn("setting O_NONBLOCK"), close(fd), -1;

    flags = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof flags);
    setsockopt(fd, SOL_SOCKET, SO_LINGER,   &linger, sizeof linger);

    saddr.sin_family = AF_INET;
    saddr.sin_port = port;
    saddr.sin_addr = host_addr;
    r = bind(fd, (const struct sockaddr *) &saddr, sizeof(saddr));
    if (r == -1) return warn("bind"), close(fd), -1;

    return fd;
}

int
make_tcp_listen_socket(struct in_addr host_addr, int port)
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
    saddr.sin_port = port;
    saddr.sin_addr = host_addr;
    r = bind(fd, (const struct sockaddr *) &saddr, sizeof(saddr));
    if (r == -1) return warn("bind"), close(fd), -1;

    r = listen(fd, 1024); // 1024 == size of backlog of pending connections
    if (r == -1) return warn("listen"), close(fd), -1;

    return fd;
}

static void
net_udp_discard(int fd)
{
    char buf;

    // Discard the packet. We don't care if there is an error -- there's
    // nothing to do anyway.
    recvfrom(fd, &buf, 1, 1, 0, 0);
}

static void
net_udp_recv(int fd, short which, void *mgr)
{
    int size;
    ssize_t r;
    struct sockaddr src;
    struct sockaddr_in *src_in;
    socklen_t src_len = sizeof(src);
    cpkt cp;

    if (!(which & EV_READ)) return warnx("got event %d", which);

    // Get the size of the next message.
    r = ioctl(fd, FIONREAD, &size);
    if (r == -1) return warn("ioctl");

    if (size < 1) {
        warnx("message too small (%d bytes) -- discarding", size);
        return net_udp_discard(fd);
    }

    if (size >= (1 << 16)) {
        warnx("message too large (%d bytes) -- discarding", size);
        return net_udp_discard(fd);
    }

    cp = make_cpkt(size);
    if (!cp) return warnx("make_cpkt");

    r = recvfrom(fd, cp->data, size, 0, &src, &src_len);
    if (r == -1) {
        if (errno != EAGAIN) warn("recvfrom");
        free(cp);
        return;
    }

    if (r != size) warnx("read only %zd out of %d bytes", r, size);

    if (src.sa_family != AF_INET) {
        warnx("not an ipv4 address");
        free(cp);
        return;
    }

    src_in = (struct sockaddr_in *) &src;
    cp->remote_addr = src_in->sin_addr.s_addr;
    cp->remote_port = src_in->sin_port;
    peer p = manager_get_peer(mgr, cp->remote_addr, cp->remote_port);
    peer_touch(p);
    cpkt_handle(cp, p);
    free(cp);
}

static void
net_udp_send(int fd, short which, void *mgr)
{
    ssize_t r;
    struct msghdr msg = { 0, /* 0... */ };
    struct sockaddr_in dst = { 0, /* 0... */ };

    while (manager_out_any(mgr)) {
        cpkt c = manager_out_remove(mgr);

        dst.sin_family = AF_INET;
        dst.sin_port = c->remote_port;
        dst.sin_addr.s_addr = c->remote_addr;

        struct iovec iov[1] = { {0,}, /* {0,}... */ };

        iov[0].iov_len = c->size;
        iov[0].iov_base = cpkt_body(c);

        msg.msg_name = &dst;
        msg.msg_namelen = sizeof(dst);
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;

        warnx("sending %d bytes", c->size);
        r = sendmsg(udp_socket, &msg, MSG_CONFIRM|MSG_DONTWAIT);

        if (r == -1) {
            int e = errno;
            if (e == EAGAIN) { // There was not enough buffer space.
                manager_out_pushback(mgr, c);
                break;
            }
            cpkt_error(c, e);
        }

        free(c);
    }
}

static void
net_udp_handle(int fd, short which, void *mgr)
{
    if (which & EV_READ) net_udp_recv(fd, which, mgr);
    if (which & EV_WRITE) net_udp_send(fd, which, mgr);
    prot_work(mgr);
    net_update_ev(mgr);
}

void
net_init(struct in_addr host_addr, int udp_port, manager mgr)
{
    int r;
    struct evhttp *ev_http;

    // First compute our own key.
    key_for_peer(mgr->key, host_addr.s_addr, udp_port);

    ev_base = event_base_new();
    if (!ev_base) {
        warn("event_base_new");
        exit(2);
    }

    udp_socket = make_udp_listen_socket(host_addr, udp_port);
    if (udp_socket == -1) {
        warnx("could not open udp port %d", udp_port);
        exit(2);
    }

    udp_ev = event_new(ev_base, udp_socket, 0, net_udp_handle, mgr);
    if (!udp_ev) {
        warnx("event_new: could not make udp event");
        exit(2);
    }

    if (mgr->memcache_port) {
        memcache_socket = make_tcp_listen_socket(host_addr, mgr->memcache_port);
        if (memcache_socket == -1) {
            warnx("could not open memcache port %d", mgr->memcache_port);
            exit(2);
        }
    }

    if (mgr->http_port) {
        ev_http = evhttp_new(ev_base);

        // evhttp_bind_socket expects its PORT param in host byte order. Sigh.
        r = evhttp_bind_socket(ev_http, "0.0.0.0", ntohs(mgr->http_port));
        // This return value is undocumented (!), but this seems to work.
        if (r == -1) {
            warn("evhttp_bind_socket");
            warnx("could not open port %d", mgr->http_port);
            exit(3);
        }

        evhttp_set_gencb(ev_http, http_handle_generic, mgr);
        evhttp_set_cb(ev_http, "/", http_handle_root, mgr);
    }

}

void
net_update_ev(manager mgr)
{
    int r;
    struct timeval tv = { 0, /* 0... */ };

    // Just in case.
    r = event_del(udp_ev);
    if (r == -1) return warnx("event_del");

    short events = EV_READ;
    if (manager_out_any(mgr)) events |= EV_WRITE;

    event_assign(udp_ev, ev_base, udp_socket, events, net_udp_handle, mgr);

    tv.tv_sec = 0;
    tv.tv_usec = (10 + (random() % 10)) * 1000; // 10-20ms

    r = event_add(udp_ev, &tv);
    if (r == -1) return warnx("event_add: could not add udp event");
}
