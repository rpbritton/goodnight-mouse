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

#include "config.h"

// todo: overhaul, make configurable at runtime.
// use https://developer.gnome.org/glib/stable/glib-Key-value-file-parser.html for config
// use getopt.h for arguments

static const gchar *css1 = ".overlay_window {"
                           "    background-color: rgba(100%, 0%, 0%, 0.05);"
                           "}";

static const gchar *css2 = ".tag_label {"
                           "    background-color: #000;"
                           "    font-family: 'IBM Plex Mono', monospace;"
                           "    font-weight: bold;"
                           "    font-size: 14px;"
                           "    border: 1px solid #FFF;"
                           "    padding: 1px 3px;"
                           "    border-radius: 3px;"
                           "}"
                           ""
                           ".tag_character {"
                           "    color: #0F0;"
                           "}"
                           ""
                           ".tag_character_active {"
                           "    color: #00F;"
                           "}";

Config *config_parse(int argc, char **argv)
{
    Config *config = g_new(Config, 1);

    config->run_once = FALSE;
    config->log_verbose = TRUE;

    config->app.foreground.codes.keys = g_array_new(FALSE, FALSE, sizeof(guint));
    guint keys[] = {GDK_KEY_e, GDK_KEY_s, GDK_KEY_n, GDK_KEY_t,
                    GDK_KEY_i, GDK_KEY_r, GDK_KEY_o, GDK_KEY_a};
    g_array_append_vals(config->app.foreground.codes.keys, keys, 8);
    config->app.foreground.codes.no_repeat = TRUE;

    GtkCssProvider *css_provider1 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider1, css1, -1, NULL);
    config->app.foreground.overlay.styling = GTK_STYLE_PROVIDER(css_provider1);

    GtkCssProvider *css_provider2 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider2, css2, -1, NULL);
    config->app.foreground.codes.tag.styling = GTK_STYLE_PROVIDER(css_provider2);
    config->app.foreground.codes.tag.alignment_horizontal = GTK_ALIGN_START;
    config->app.foreground.codes.tag.alignment_vertical = GTK_ALIGN_CENTER;

    config->app.background.trigger_id = GDK_KEY_v;
    config->app.background.trigger_modifiers = GDK_SUPER_MASK;

    return config;
}

void config_destroy(Config *config)
{
    g_array_unref(config->app.foreground.codes.keys);

    g_object_unref(config->app.foreground.overlay.styling);
    g_object_unref(config->app.foreground.codes.tag.styling);

    g_free(config);
}