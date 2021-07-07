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

#ifndef CB504225_79CB_4844_9848_73E9861C2CE1
#define CB504225_79CB_4844_9848_73E9861C2CE1

#include <glib.h>

typedef struct Tag
{
    GArray *code;
    // x y pos
} Tag;

Tag *tag_new();
void tag_destroy(gpointer tag);
GArray *tag_get_code(Tag *tag);
void tag_set_code(Tag *tag, GArray *code);
//void tag_set_window(Tag *tag, GTKWINDOW);

#endif /* CB504225_79CB_4844_9848_73E9861C2CE1 */
