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

#include "config.h"

#if HAVE_STDINT_H
# include <stdint.h>
#endif /* else we get int types from config.h */

#include <stdlib.h>
#include <netinet/in.h>

#define CEILDIV(num, den) (1 + (((num) - 1) / (den)))

#define CEILDIVL(num, denexp) (1 + ((num - 1) >> denexp))

#define ROUND_UP_TO_POWER_OF_TWO(x, k) (CEILDIVL(x, k) << k)

#define ALIGN(a,t) ((t) ROUND_UP_TO_POWER_OF_TWO(((size_t) a), 3))

#define min(a,b) ((a)<(b)?(a):(b))

// All times are in usec.
#define SECOND (1000000)

int startswith(const char *haystack, const char *needle);

void raw_warn(const char *fmt, ...);
void raw_warnx(const char *fmt, ...);

void raw_err(int eval, const char *fmt, ...);
void raw_errx(int eval, const char *fmt, ...);

extern char *progname;
extern int util_id;

#define warn(fmt, args...) raw_warn("%s:%d in %s: " fmt, \
                                 __FILE__, __LINE__, __func__, ##args)
#define warnx(fmt, args...) raw_warnx("%s:%d in %s: " fmt, \
                                   __FILE__, __LINE__, __func__, ##args)

#define err(eval, fmt, args...) raw_err(eval, "%s:%d in %s: " fmt, \
                                        __FILE__, __LINE__, __func__, ##args)
#define errx(eval, fmt, args...) raw_errx(eval, "%s:%d in %s: " fmt, \
                                          __FILE__, __LINE__, __func__, ##args)

uint64_t now_usec(void);
uint64_t int_from_timeval(struct timeval *tv);
void timeval_from_int(struct timeval *tv, uint64_t t);

void key_for_peer(uint32_t key[3], in_addr_t addr, uint16_t port);

#endif /*util_h*/
