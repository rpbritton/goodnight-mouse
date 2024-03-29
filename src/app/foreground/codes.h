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

#include "codes_config.h"

#include "tag.h"

// a tag generator that assignes unique codes from the given set of keys
typedef struct Codes
{
    GArray *code;

    TagConfig *tag_config;

    GArray *keys;
    GArray *code_prefix;
    gint key_index;
    gboolean consecutive_keys;

    GList *tags;
    GHashTable *tags_used;
    GList *tags_unused;
} Codes;

Codes *codes_new(CodesConfig *config);
void codes_destroy(Codes *codes);
Tag *codes_allocate(Codes *codes);
void codes_deallocate(Codes *codes, Tag *tag);
void codes_add_key(Codes *codes, guint key);
void codes_pop_key(Codes *codes);
Tag *codes_matched_tag(Codes *codes);

#endif /* B10FD127_9857_4FE9_AF02_AB3EC418F0FF */
