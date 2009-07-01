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

#include "util.h"

char *progname = 0; /* defined as extern in util.h */

static void
raw_vwarnx(const char *err, const char *fmt, va_list args)
{
    fprintf(stderr, "%s: ", progname);
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
