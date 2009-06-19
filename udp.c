// udp.c -- udp handlers

#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event2/event.h>

#include "udp.h"
#include "util.h"

static void
udp_handle_ping(cpkt cp, struct sockaddr_in *src)
{
    // FIXME: stub
    raw_warnx("got ping");
}

static void
udp_handle_pong(cpkt cp, struct sockaddr_in *src)
{
    // FIXME: stub
    raw_warnx("got pong");
}

static void
udp_handle_link(cpkt cp, struct sockaddr_in *src)
{
    // FIXME: stub
    raw_warnx("got link");
}

typedef void(*udp_cpkt_handle_fn)(cpkt, struct sockaddr_in *);

#define KNOWN_TYPES 3
static udp_cpkt_handle_fn types[3] = {
    udp_handle_ping,
    udp_handle_pong,
    udp_handle_link,
};

static void
udp_discard(fd)
{
    char buf;

    // Discard the packet. We don't care if there is an error -- there's
    // nothing to do anyway.
    recvfrom(fd, &buf, 1, 1, 0, 0);
}

void
udp_recv(int fd, short which, void *mgr)
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

    if (size < sizeof(struct cpkt)) {
        warnx("message too small (%d bytes) -- discarding", size);
        return udp_discard(fd);
    }

    cp = malloc(size);
    if (!cp) return warn("malloc");

    r = recvfrom(fd, cp, size, 0, &src, &src_len);
    if (r == -1) {
        if (errno != EAGAIN) warn("recvfrom");
        free(cp);
        return;
    }

    if (r != size) warnx("read only %zd out of %d bytes", r, size);

    if (src.sa_family != AF_INET) {
        warnx("not an internet address");
        free(cp);
        return;
    }

    src_in = (struct sockaddr_in *) &src;

    if (cp->type >= KNOWN_TYPES) {
        warnx("unknown message type %d -- discarding", cp->type);
        free(cp);
        return;
    }

    types[cp->type](cp, src_in);
    free(cp);
}
