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

// use https://developer.gnome.org/glib/stable/glib-Key-value-file-parser.html for config
// use getopt.h for arguments

Config *config_parse(int argc, char **argv)
{
    Config *config = g_new(Config, 1);

    config->run_once = FALSE;
    config->log_verbose = TRUE;

    config->app.foreground.keys = g_array_new(FALSE, FALSE, sizeof(guint));
    guint keys[] = {GDK_KEY_a, GDK_KEY_b, GDK_KEY_c};
    g_array_append_vals(config->app.foreground.keys, keys, 3);

    config->app.background.trigger_id = GDK_KEY_v;
    config->app.background.trigger_modifiers = GDK_SUPER_MASK;

    return config;
}

void config_destroy(Config *config)
{
    g_array_unref(config->app.foreground.keys);

    g_free(config);
}