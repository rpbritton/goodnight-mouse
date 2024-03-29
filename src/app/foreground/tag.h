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

#ifndef D8EAD49E_03F6_46EE_9DA3_3605763E815D
#define D8EAD49E_03F6_46EE_9DA3_3605763E815D

#include <atspi/atspi.h>
#include <gtk/gtk.h>

#include "tag_config.h"

// a tag that can show a code as a gtk widget over an accessible
typedef struct Tag
{
    GArray *code;
    gint match_index;

    AtspiAccessible *accessible;

    gboolean shifted;

    GtkLayout *parent;
    gint window_x, window_y;

    GtkWidget *wrapper;
    GtkWidget *label;
    GArray *characters;
} Tag;

Tag *tag_new(TagConfig *config);
void tag_destroy(Tag *tag);

void tag_set_accessible(Tag *tag, AtspiAccessible *accessible);
void tag_unset_accessible(Tag *tag);

void tag_shifted(Tag *tag, gboolean shifted);

void tag_show(Tag *tag, GtkLayout *parent, gint window_x, gint window_y);
void tag_hide(Tag *tag);

void tag_set_code(Tag *tag, GArray *code);
GArray *tag_get_code(Tag *tag);
void tag_unset_code(Tag *tag);
gboolean tag_apply_code(Tag *tag, GArray *code);
gboolean tag_matches_code(Tag *tag);

#endif /* D8EAD49E_03F6_46EE_9DA3_3605763E815D */
