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

#ifndef B10FD127_9857_4FE9_AF02_AB3EC418F0FF
#define B10FD127_9857_4FE9_AF02_AB3EC418F0FF

#include <glib.h>
#include <gdk/gdk.h>

#include "control.h"

typedef struct CodesConfig
{
    GArray *keys;
} CodesConfig;

typedef struct Codes
{
    GArray *keys;
    GArray *code_prefix;
    gint key_index;

    GList *mappings;
} Codes;

Codes *codes_new(CodesConfig *config);
void codes_destroy(Codes *codes);
void codes_reset(Codes *codes);
void codes_control_add(Codes *codes, Control *control);
void codes_control_remove(Codes *codes, Control *control);
void codes_key_add(Codes *codes, guint key);
void codes_key_remove(Codes *codes);

#endif /* B10FD127_9857_4FE9_AF02_AB3EC418F0FF */
