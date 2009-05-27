// cubbyd -- The storage node daemon for Cubby.

// Cubby provides simple, fast, scalable storage for static blobs.

#include "net.h"
#include "prot.h"
#include "store.h"

int
main(int argc, char **argv)
{

    //opts;

    store_init(); // Read and index the files

    prot_init();
    net_init();

    prot_bootstrap();
    net_loop();
    return 1;
}
