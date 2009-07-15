// util.c -- util functions

/* Copyright (C) 2009 Keith Rarick and Tuenti Technologies, Inc.

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "sha512.h"
#include "util.h"

char *progname = 0; /* defined as extern in util.h */
int util_id = 0; /* defined as extern in util.h */

static void
raw_vwarnx(const char *err, const char *fmt, va_list args)
{
    fprintf(stderr, "%s:%d: ", progname, util_id);
    if (fmt) {
        vfprintf(stderr, fmt, args);
        if (err) fprintf(stderr, ": %s", strerror(errno));
    }
    fputc('\n', stderr);
}

void
raw_warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    raw_vwarnx(strerror(errno), fmt, args);
    va_end(args);
}

void
raw_warnx(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    raw_vwarnx(NULL, fmt, args);
    va_end(args);
}

void
raw_err(int eval, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    raw_vwarnx(strerror(errno), fmt, args);
    va_end(args);
    exit(eval);
}

void
raw_errx(int eval, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    raw_vwarnx(NULL, fmt, args);
    va_end(args);
    exit(eval);
}

int
startswith(const char *haystack, const char *needle)
{
    if (!haystack || !needle) return 0;
    for (;; haystack++, needle++) {
        if (!*needle) return 1;
        if (*haystack != *needle) return 0;
    }
}

uint64_t
int_from_timeval(struct timeval *tv)
{
    return ((uint64_t) tv->tv_sec) * SECOND + tv->tv_usec;
}

void
timeval_from_int(struct timeval *tv, uint64_t t)
{
    tv->tv_sec = t / SECOND;
    tv->tv_usec = t % SECOND;
}

uint64_t
now_usec(void)
{
    int r;
    struct timeval tv;

    r = gettimeofday(&tv, 0);
    if (r != 0) return warnx("gettimeofday"), -1; // can't happen

    return int_from_timeval(&tv);
}

/* Expects ADDR and PORT to be in network order (big-endian). */
/* Fills in 12 bytes starting at KEY. */
void
key_for_peer(uint32_t *key, in_addr_t addr, uint16_t port)
{
    struct __attribute__ ((__packed__)) {
        uint32_t addr; // always big-endian
        uint16_t port; // always big-endian
    } buf = { addr, port };
    sha512((char *) &buf, sizeof(buf), key, sizeof(uint32_t) * 3);
}
