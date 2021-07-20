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

#include "styler.h"

// start and create a new styler
Styler *styler_start()
{
    Styler *styler = g_new(Styler, 1);
    styler->css = g_strdup("");
    return styler;
}

// cleans up the styler and returns a css provider
GtkCssProvider *styler_finish(Styler *styler)
{
    // create provider
    GtkCssProvider *styling = gtk_css_provider_new();
    gtk_css_provider_load_from_data(styling, styler->css, -1, NULL);

    // free styler
    g_free(styler->css);
    g_free(styler);

    return styling;
}

// adds a css property to the given class
void styler_add(Styler *styler, gchar *css_class, gchar *property, gchar *value)
{
    gchar *old_css = styler->css;
    styler->css = g_strdup_printf("%s.%s{%s:%s;}", styler->css, css_class, property, value);
    g_free(old_css);
}