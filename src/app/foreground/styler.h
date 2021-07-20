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

#ifndef B576DABE_28FE_48DA_99B7_AE95ADCD4897
#define B576DABE_28FE_48DA_99B7_AE95ADCD4897

#include <glib.h>
#include <gtk/gtk.h>

// tool for creating a css provider from added properties
typedef struct Styler
{
    gchar *css;
} Styler;

Styler *styler_start();
GtkCssProvider *styler_finish(Styler *styler);
void styler_add(Styler *styler, gchar *css_class, gchar *property, gchar *value);

#endif /* B576DABE_28FE_48DA_99B7_AE95ADCD4897 */
