// cubbyd -- The storage node daemon for Cubby.

// Cubby provides simple, fast, scalable storage for static blobs.

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <event2/event.h>

#include "net.h"
#include "prot.h"
#include "bundle.h"
#include "util.h"

#define DEFAULT_MEMCACHE_PORT 11211
#define DEFAULT_HTTP_PORT 80

static struct in_addr host_addr = {};
static int memcache_port = DEFAULT_MEMCACHE_PORT,
           http_port = DEFAULT_HTTP_PORT;

static void
usage(char *msg, char *arg)
{
    if (arg) {
        raw_warnx("%s: %s", msg, arg);
    } else if (msg) {
        raw_warnx("%s", msg);
    }
    fprintf(stderr, "Use: %s [OPTIONS]\n"
            "\n"
            "Options:\n"
            " -b FILE  use this bundle file (may be given more than once)\n"
            " -m PORT  memcache, listen on port PORT (default %d)\n"
            " -p PORT  HTTP, listen on port PORT (default %d)\n"
            " -i       initialize bundles (negates -I)\n"
            " -I       don't initalize bundles (default; negates -i)\n"
            " -v       show version information\n"
            " -h       show this help\n",
            progname, DEFAULT_MEMCACHE_PORT, DEFAULT_HTTP_PORT);
    exit(arg ? 5 : 0);
}

static char *
require_arg(char *opt, char *arg)
{
    if (!arg) usage("option requires an argument", opt);
    return arg;
}

static int
parse_port(char *portstr)
{
    int port;
    char *end;

    errno = 0;
    port = strtol(portstr, &end, 10);
    if (end == portstr) usage("invalid port", portstr);
    if (end[0] != 0) usage("invalid port", portstr);
    if (errno) usage("invalid port", portstr);

    return port;
}

static void
opts(char **argv)
{
    char *opt;

    while ((opt = *argv++)) {
        if (opt[0] != '-') usage("unknown argument", opt);
        if (opt[1] == 0 || opt[2] != 0) usage("unknown option", opt);
        switch (opt[1]) {
            case 'b':
                add_bundle(*argv++);
                break;
            case 'i':
                initialize_bundles = 1;
                break;
            case 'I':
                initialize_bundles = 0;
                break;
            case 'm':
                memcache_port = parse_port(require_arg("-m", *argv++));
                break;
            case 'p':
                http_port = parse_port(require_arg("-p", *argv++));
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
    spht dir;
    struct event_base *ev_base;

    host_addr.s_addr = INADDR_ANY;
    progname = *argv;
    opts(argv + 1);

    dir = make_spht(0);

    r = bundles_init(dir); // Read and index the files
    if (r == -1) usage("Try the -b option", 0);
    if (r == -2) return warnx("cannot continue"), 2;

    prot_init();
    ev_base = event_base_new();
    if (!ev_base) return warn("event_base_new"), 2;
    net_init(ev_base, host_addr, memcache_port, http_port);

    prot_bootstrap();
    r = net_loop(ev_base);
    if (r == -1) return warnx("error in main driver loop"), 19;

    return 0;
}
