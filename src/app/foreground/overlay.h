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

#ifndef CD11B90E_5DEF_4A3C_9FA5_7626C9545641
#define CD11B90E_5DEF_4A3C_9FA5_7626C9545641

#include <glib.h>
#include <gtk/gtk.h>
#include <atspi/atspi.h>

#include "tag.h"

#define OVERLAY_WINDOW_TITLE "goodnight_mouse"
#define OVERLAY_CSS_CLASS "overlay_window"

typedef struct OverlayConfig
{
    GtkStyleProvider *styling;
} OverlayConfig;

typedef struct Overlay
{
    AtspiAccessible *window;
    GHashTable *tags;

    GtkWidget *overlay;
    GtkWidget *container;

    guint refresh_source_id;
} Overlay;

Overlay *overlay_new(OverlayConfig *config);
void overlay_destroy(Overlay *overlay);
void overlay_show(Overlay *overlay, AtspiAccessible *window);
void overlay_hide(Overlay *overlay);
void overlay_add(Overlay *overlay, Tag *tag);
void overlay_remove(Overlay *overlay, Tag *tag);

#endif /* CD11B90E_5DEF_4A3C_9FA5_7626C9545641 */
