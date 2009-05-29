// bundle.c -- A disk file that contains cubby data

#include <stdlib.h>

#include "bundle.h"
#include "util.h"

// This points to an array of structs, not a single one.
static struct bundle *all_bundles = 0;
static int nbundles = 0;

/* returns 0 on success, -1 on error */
int
add_bundle(char *name)
{
    struct bundle *new_bundles;

    new_bundles = realloc(all_bundles, sizeof(struct bundle) * (nbundles + 1));
    if (!new_bundles) return warn("realloc"), -1;

    all_bundles = new_bundles;

    all_bundles[nbundles].name = name;
    all_bundles[nbundles].base = 0;
    all_bundles[nbundles].nregions = 0;

    ++nbundles;
    return 0;
}
