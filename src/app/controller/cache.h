/**
 * Copyright (C) 2021 Ryan Britton
 *
 * This file is part of Goodnight Mouse.
 *
 * Goodnight Mouse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Goodnight Mouse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Goodnight Mouse.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CFCF927F_0647_4D08_A205_B92BCBF48C80
#define CFCF927F_0647_4D08_A205_B92BCBF48C80

#include <glib.h>
#include <atspi/atspi.h>

typedef struct Cache
{
    GHashTable *windows;
    AtspiMatchRule *match_rule;
} Cache;

Cache *cache_new();
void cache_destroy(Cache *cache);
GArray *cache_list(Cache *cache, AtspiAccessible *window);
void cache_add(Cache *cache, AtspiAccessible *accessible);
void cache_add_window(Cache *cache, AtspiAccessible *window);
void cache_remove(Cache *cache, AtspiAccessible *accessible);

#endif /* CFCF927F_0647_4D08_A205_B92BCBF48C80 */
