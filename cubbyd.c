// cubbyd -- The storage node daemon for Cubby.

// Cubby provides simple, fast, scalable storage for static blobs.

#include "config.h"

#include <stdlib.h>
#include <stdio.h>

#include "net.h"
#include "prot.h"
#include "store.h"
#include "bundle.h"
#include "util.h"

static void
usage(char *msg, char *arg)
{
    if (arg) raw_warnx("%s: %s", msg, arg);
    fprintf(stderr, "Use: %s [OPTIONS]\n"
            "\n"
            "Options:\n"
            " -b FILE  use this bundle file (may be given more than once)\n"
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
    progname = *argv;
    opts(argv + 1);

    store_init(); // Read and index the files

    prot_init();
    net_init();

    prot_bootstrap();
    net_loop();
    return 1;
}
