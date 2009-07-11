/* arr.c - resizable multiset implementation */

/* Copyright (C) 2008 Keith Rarick and Philotic Inc.

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

#include <string.h>
#include <stdlib.h>

#include "arr.h"
#include "util.h"

void
arr_init(arr a, arr_event_fn after_insert, arr_event_fn after_remove)
{
    a->used = a->cap = a->last = 0;
    a->items = 0;
    a->after_insert = after_insert;
    a->after_remove = after_remove;
}

static void
grow(arr a)
{
    void **nitems;
    size_t ncap = (a->cap << 1) ? : 1;

    nitems = malloc(ncap * sizeof(void *));
    if (!nitems) return;

    memcpy(nitems, a->items, a->used * sizeof(void *));
    free(a->items);
    a->items = nitems;
    a->cap = ncap;
}

int
arr_append(arr a, void *item)
{
    if (a->used >= a->cap) grow(a);
    if (a->used >= a->cap) return 0;

    a->items[a->used] = item;
    if (a->after_insert) a->after_insert(a, item, a->used);
    ++a->used;
    return 1;
}

static int
arr_delete(arr a, size_t i)
{
    void *item;

    if (i >= a->used) return 0;
    item = a->items[i];
    a->items[i] = a->items[--a->used];

    /* it has already been removed now */
    if (a->after_remove) a->after_remove(a, item, i);
    return 1;
}

void
arr_clear(arr a)
{
    while (arr_delete(a, 0));
    free(a->items);
    arr_init(a, a->after_insert, a->after_remove);
}

int
arr_remove(arr a, void *item)
{
    size_t i;

    for (i = 0; i < a->used; i++) {
        if (a->items[i] == item) return arr_delete(a, i);
    }
    return 0;
}

int
arr_contains(arr a, void *item)
{
    size_t i;

    for (i = 0; i < a->used; i++) {
        if (a->items[i] == item) return 1;
    }
    return 0;
}

void *
arr_take(arr a)
{
    void *item;

    if (!a->used) return NULL;

    a->last = a->last % a->used;
    item = a->items[a->last];
    arr_delete(a, a->last);
    ++a->last;
    return item;
}

void
arr_filter(arr a, arr_event_fn f)
{
    for (size_t i = 0; i < a->used;) {
        if (f(a, a->items[i], i)) {
            i++;
        } else {
            arr_delete(a, i);
        }
    }
}
