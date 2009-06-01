// cubbyd -- The storage node daemon for Cubby.

// Cubby provides simple, fast, scalable storage for static blobs.

#include "config.h"

#include <stdlib.h>
#include <stdio.h>

#include "net.h"
#include "prot.h"
#include "bundle.h"
#include "util.h"

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
            " -i       initialize bundles (negates -I)\n"
            " -I       don't initalize bundles (default; negates -i)\n"
            " -v       show version information\n"
            " -h       show this help\n",
            progname);
    exit(arg ? 5 : 0);
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

    progname = *argv;
    opts(argv + 1);

    dir = make_spht(0);

    r = bundles_init(dir); // Read and index the files
    if (r == -1) usage("Try the -b option", 0);
    if (r == -2) return warnx("cannot continue"), 2;

    prot_init();
    net_init();

    prot_bootstrap();
    net_loop();
    return 1;
}
