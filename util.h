// util.h -- util functions

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

#ifndef util_h
#define util_h

#include "stdlib.h"

#define CEILDIV(num, den) (1 + (((num) - 1) / (den)))

#define CEILDIVL(num, denexp) (1 + ((num - 1) >> denexp))

#define ROUND_UP_TO_POWER_OF_TWO(x, k) (CEILDIVL(x, k) << k)

#define ALIGN(a,t) ((t) ROUND_UP_TO_POWER_OF_TWO(((size_t) a), 3))

#define min(a,b) ((a)<(b)?(a):(b))

int startswith(const char *haystack, const char *needle);

void raw_warn(const char *fmt, ...);
void raw_warnx(const char *fmt, ...);

extern char *progname;

#define warn(fmt, args...) raw_warn("%s:%d in %s: " fmt, \
                                 __FILE__, __LINE__, __func__, ##args)
#define warnx(fmt, args...) raw_warnx("%s:%d in %s: " fmt, \
                                   __FILE__, __LINE__, __func__, ##args)

#endif /*util_h*/
