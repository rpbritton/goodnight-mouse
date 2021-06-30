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

#include "codes.h"

static guint key_from_index(Codes *codes, guint index);
static guint key_to_index(Codes *codes, guint key);

Codes *codes_new(CodesConfig config)
{
    Codes *codes = g_new(Codes, 1);

    // add members
    codes->keys = g_array_copy(config.keys);

    return codes;
}

void codes_destroy(Codes *codes)
{
    g_free(codes);
}

void codes_add_control(Codes *codes, Control *control)
{
}

void codes_remove_control(Codes *codes, Control *control)
{
}

void codes_add_key(Codes *codes, guint keyval)
{
}

void codes_remove_key(Codes *codes)
{
}

static guint key_from_index(Codes *codes, guint index)
{
    if (index >= codes->keys->len)
        return -1;
    return g_array_index(codes->keys, guint, index);
}

static guint key_to_index(Codes *codes, guint key)
{
    for (gint index = 0; index < codes->keys->len; index++)
        if (key == g_array_index(codes->keys, guint, index))
            return index;
    return -1;
}
