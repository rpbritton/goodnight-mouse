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

Config *config_new(int argc, char **argv)
{
    // todo: set with arguments
    gchar *config_file = "./examples/config/goodnight_mouse.cfg";

    // get the key file
    GKeyFile *key_file = g_key_file_new();
    GError *error = NULL;
    g_key_file_load_from_file(key_file, config_file, G_KEY_FILE_NONE, &error);
    if (error)
    {
        g_warning("config: Failed to load config file: %s", error->message);
        g_error_free(error);
        return NULL;
    }
    g_key_file_set_list_separator(key_file, ',');

    // todo: override key file parameters with arguments
    // (causes a bit of a loop problem with config being an argument)

    // create config
    Config *config = g_new0(Config, 1);
    gboolean config_valid = TRUE;

    // get once
    config->once = FALSE;

    // get verbose
    config->verbose = TRUE;

    // get app
    config->app = app_new_config(key_file);
    if (!config->app)
        config_valid = FALSE;

    // todo: free key file

    // return
    if (!config_valid)
    {
        config_destroy(config);
        return NULL;
    }
    return config;
}

void config_destroy(Config *config)
{
    if (!config)
        return;

    app_destroy_config(config->app);

    g_free(config);
}