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

#include "tag.h"

Tag *tag_new()
{
    Tag *tag = g_new(Tag, 1);

    tag->code = g_array_new(FALSE, FALSE, sizeof(guint));

    return tag;
}

void tag_destroy(gpointer tag_ptr)
{
    Tag *tag = tag_ptr;

    g_array_unref(tag->code);

    g_free(tag);
}

GArray *tag_get_code(Tag *tag)
{
    return tag->code;
}

void tag_set_code(Tag *tag, GArray *code)
{
    g_array_unref(tag->code);
    tag->code = code;
}
