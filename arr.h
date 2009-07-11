/* arr.h - resizable multiset header */

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

#ifndef arr_h
#define arr_h

#include <string.h>

typedef struct arr *arr;

typedef int(*arr_event_fn)(arr a, void *item, size_t index);

struct arr {
    size_t used, cap, last;
    void **items;
    arr_event_fn after_insert, after_remove;
};

void arr_init(arr a, arr_event_fn after_insert, arr_event_fn after_remove);
void arr_clear(arr a);
int arr_append(arr a, void *item);
int arr_remove(arr a, void *item);
int arr_contains(arr a, void *item);
void *arr_take(arr a);

/* It is okay for callback f to call arr_append or arr_clear, but not
   arr_remove or arr_take. */
void arr_filter(arr a, arr_event_fn f);

#endif /*arr_h*/
