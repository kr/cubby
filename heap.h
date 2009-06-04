/* heap.h - binary heap header */

/* Copyright (C) 2007 Keith Rarick and Philotic Inc.

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

#ifndef heap_h
#define heap_h

typedef struct heap *heap;
typedef int(*cmp_fn)(void *, void *);

struct heap {
    unsigned int cap;
    unsigned int used;
    cmp_fn cmp;
    void **heap;
};

/* initialize a heap */
void heap_init(heap hp, cmp_fn cmp);

void heap_clear(heap hp);

/* return 1 if the object was successfully inserted, else 0 */
int heap_give(heap hp, void *x);

/* return an object if the heap contains an object, else NULL */
void *heap_take(heap hp);

/* return an object if the heap contains an object, else NULL */
void *heap_peek(heap hp);

#endif /*heap_h*/
