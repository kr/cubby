// net.c -- generic networking

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "util.h"

static int udp_socket;

static int
dispatch()
{
    return 0;
}

static int
net_read(short events) {
    int r, size;

    if (events & POLLIN) {
        r = ioctl(udp_socket, FIONREAD, &size);
        if (r == -1) return warn("ioctl"), -1;

        if (size == 0) return warnx("no pending datagram"), -1; // can't happen

        {
            char buf[size];
            struct sockaddr src_addr;
            socklen_t addrlen = sizeof(src_addr);
            ssize_t s;

            s = recvfrom(udp_socket, buf, size, MSG_DONTWAIT, &src_addr,
                    &addrlen);
            if (s == -1) return warn("recvfrom"), -1;
            if (s > 0) return dispatch(size, &buf, &src_addr, &addrlen);
            return warnx("got EOF on udp socket"), -1; // can't happen
        }
    }

    return 0;
}

static int
once() {
    int r;
    struct pollfd pfd;

    pfd.fd = udp_socket;
    pfd.events = POLLIN | POLLOUT;

    r = poll(&pfd, 1, -1);
    if (r > 0) return net_read(pfd.revents);
    if (r < 0) return warn("poll"), -1;

    // r == 0 means timeout
    warnx("can't happen"), exit(2);
}

int
net_loop()
{
    int r;

    for (;;) {
        r = once();
        if (r == -1) return warnx("once"), -1;
    }
}

void
net_init()
{
    int r;
    struct sockaddr_in saddr = {};
    uint16_t port = 10101;

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket == -1) warn("socket"), exit(2);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = INADDR_ANY;

    r = bind(udp_socket, (const struct sockaddr *) &saddr, sizeof(saddr));
    if (r == -1) warn("bind"), exit(2);
}
