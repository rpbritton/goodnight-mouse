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

#ifndef AD6E8486_D4A4_42A2_93B3_1908FF782248
#define AD6E8486_D4A4_42A2_93B3_1908FF782248

#include <glib.h>
#include <gtk/gtk.h>

#define OVERLAY_CSS_CLASS "overlay_window"

// overlay that shows on top of the focus window and contains tags
typedef struct OverlayConfig
{
    GtkStyleProvider *styling;
} OverlayConfig;

OverlayConfig *overlay_new_config(GKeyFile *key_file);
void overlay_destroy_config(OverlayConfig *config);

#endif /* AD6E8486_D4A4_42A2_93B3_1908FF782248 */
