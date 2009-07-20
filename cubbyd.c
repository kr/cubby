// cubbyd -- The storage node daemon for Cubby.

// Cubby provides simple, fast, scalable storage for static blobs.

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <event2/event.h>

#include "net.h"
#include "prot.h"
#include "bundle.h"
#include "manager.h"
#include "peer.h"
#include "util.h"

#define DEFAULT_MEMCACHE_PORT 11211
#define DEFAULT_HTTP_PORT 80
#define DEFAULT_UDP_PORT 20202

static struct in_addr host_addr = {};
static int udp_port;
static int use_net = 1;

static void
usage(const char *msg, const char *arg)
{
    if (arg) {
        raw_warnx("%s: %s", msg, arg);
    } else if (msg) {
        raw_warnx("%s", msg);
    }
    fprintf(stderr, "Use: %s [OPTIONS]\n"
      "\n"
      "Options:\n"
      " -f FILE        use this storage file (may be given more than once)\n"
      " -m PORT        memcache, listen on port PORT (default %d)\n"
      " -p PORT        HTTP, listen on port PORT (default %d)\n"
      " -c PORT        control protocol, listen on UDP port PORT (default %d)\n"
      " -b ADDR[:PORT] bootstrap by connecting to ADDR on PORT (default %d)\n"
      " -i             initialize storage (negates -I)\n"
      " -I             don't initalize storage (default; negates -i)\n"
      " -n             quit before any networking (useful with -i)\n"
      " -v             show version information\n"
      " -h             show this help\n",
      progname, DEFAULT_MEMCACHE_PORT, DEFAULT_HTTP_PORT,
      DEFAULT_UDP_PORT, DEFAULT_UDP_PORT);
    exit(arg ? 5 : 0);
}

static char *
require_arg(char *opt, char *arg)
{
    if (!arg) usage("option requires an argument", opt);
    return arg;
}

/* returns port in network order */
static int
parse_port(const char *portstr)
{
    int port;
    char *end;

    errno = 0;
    port = strtol(portstr, &end, 10);
    if (end == portstr) usage("invalid port", portstr);
    if (end[0] != 0) usage("invalid port", portstr);
    if (errno) usage("invalid port", portstr);

    return htons(port);
}

static void
parse_host_port(const char *arg, in_addr_t *addr, int *port, int defport)
{
    int r;
    char *portstr;
    struct in_addr addr_s;

    portstr = strchr(arg, ':');

    if (!portstr) {
        r = inet_pton(AF_INET, arg, &addr_s);
        if (!r) usage("invalid peer address", arg);

        *addr = addr_s.s_addr;
        *port = defport;
        return;
    }

    int len = portstr - arg;
    char addrstr[len + 1];

    memcpy(addrstr, arg, len);
    addrstr[len] = 0;

    r = inet_pton(AF_INET, addrstr, &addr_s);
    if (!r) usage("invalid peer address", addrstr);

    *addr = addr_s.s_addr;
    *port = parse_port(portstr + 1);
}

static void
add_bootstrap_peer(manager mgr, const char *arg)
{
    int port;
    in_addr_t addr;

    parse_host_port(arg, &addr, &port, DEFAULT_UDP_PORT);

    peer p = manager_get_peer(mgr, addr, port);
    if (!p) errx(50, "failed to allocate boostrap peer");
}

static void
opts(manager mgr, char **argv)
{
    char *opt;

    while ((opt = *argv++)) {
        if (opt[0] != '-') usage("unknown argument", opt);
        if (opt[1] == 0 || opt[2] != 0) usage("unknown option", opt);
        switch (opt[1]) {
            case 'f':
                add_bundle(mgr, require_arg("-f", *argv++));
                break;
            case 'i':
                initialize_bundles = 1;
                break;
            case 'I':
                initialize_bundles = 0;
                break;
            case 'm':
                mgr->memcache_port = parse_port(require_arg("-m", *argv++));
                break;
            case 'c':
                udp_port = parse_port(require_arg("-c", *argv++));
                break;
            case 'p':
                mgr->http_port = parse_port(require_arg("-p", *argv++));
                break;
            case 'b':
                add_bootstrap_peer(mgr, require_arg("-b", *argv++));
                break;
            case 'n':
                use_net = 0;
                break;
            case 'h':
                usage(0, 0);
            case 'v':
                puts(PACKAGE_STRING);
                exit(0);
            default:
                usage("unknown option", opt);
        }
    }
}

int
main(int argc, char **argv)
{
    int r;
    struct manager mgr = {};

    progname = *argv;
    host_addr.s_addr = INADDR_ANY;
    udp_port = htons(DEFAULT_UDP_PORT);
    mgr.memcache_port = htons(DEFAULT_MEMCACHE_PORT);
    mgr.http_port = htons(DEFAULT_HTTP_PORT);
    opts(&mgr, argv + 1);
    util_id = ntohs(udp_port);

    r = manager_init(&mgr);
    if (r == -1) return warnx("cannot continue"), 2;
    if (r == -2) usage("Try the -f option", 0);

    if (!use_net) return 0;

    // We use random(3) just to provide jitter in a few places. It's okay if we
    // don't have a very good seed here. We just want a reasonable chance it'll
    // be unique in the cluster.
    srandom(host_addr.s_addr ^ udp_port);

    prot_init();
    net_init(host_addr, udp_port, &mgr);

    prot_work(&mgr);
    r = net_loop(&mgr);
    if (r == -1) return warnx("error in main driver loop"), 19;

    return 0;
}
