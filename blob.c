// blob.c -- An individual blob on disk

#include "blob.h"
#include "util.h"

blob
blob_next(blob bl)
{
    return ALIGN(bl->data + bl->size, blob);
}

/* return 0 on success, -1 on failure */
int
blob_verify(blob bl)
{
    return 0;
}
