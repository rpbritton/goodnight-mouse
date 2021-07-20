/**
 * Copyright (C) 2021 Ryan Britton
 *
 * This file is part of goodnight_mouse.
 *
 * goodnight_mouse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * goodnight_mouse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with goodnight_mouse.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FADD6E03_18B9_4404_B009_85AF0CAD8B89
#define FADD6E03_18B9_4404_B009_85AF0CAD8B89

#include <glib.h>
#include <gtk/gtk.h>

// todo: move
#define TAG_CONTAINER_CSS_CLASS "tag_container"
#define TAG_LABEL_CSS_CLASS "tag_label"
#define TAG_CHARACTER_CSS_CLASS "tag_character"
#define TAG_CHARACTER_ACTIVE_CSS_CLASS "tag_character_active"

typedef struct TagConfig
{
    GtkStyleProvider *styling;
    GtkAlign alignment_horizontal;
    GtkAlign alignment_vertical;
} TagConfig;

TagConfig *tag_new_config(GKeyFile *key_file);
void tag_destroy_config(TagConfig *config);

#endif /* FADD6E03_18B9_4404_B009_85AF0CAD8B89 */
