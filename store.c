// store.c -- Storage

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "util.h"

#define STORE_FILE "blob.store"

static char *store_base;

static void
make_index(char *base)
{
}

static size_t
store_check_header(int fd)
{
    return 1024 * 1024;
}

void
store_init()
{
    size_t size;
    int fd, r;

    fd = open(STORE_FILE, O_RDWR|O_CREAT, 0600);
    if (fd == -1) return warn("open(%s)", STORE_FILE);

    r = fcntl(fd, F_SETFD, FD_CLOEXEC);
    if (r == -1) return warn("fcntl");

    size = store_check_header(fd);
    store_base = mmap(0, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    if (store_base == MAP_FAILED) return warn("mmap");

    make_index(store_base);
}
