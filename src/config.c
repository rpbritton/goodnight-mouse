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

#define DEFAULT_CONFIG_PATH "~/.config/goodnight_mouse/goodnight_mouse.cfg"

// create a config from the command line arguments and config file
Config *config_new(int argc, char **argv)
{
    // create config
    Config *config = g_new0(Config, 1);
    gboolean config_valid = TRUE;

    GError *error = NULL;

    // default command line arguments
    gchar *config_path = NULL;
    config->verbose = FALSE;
    config->once = FALSE;

    // add command line arguments
    GOptionContext *context = g_option_context_new(NULL);
    GOptionEntry entries[] =
        {
            {"config", 'c', 0, G_OPTION_ARG_FILENAME, &config_path, "Path to config file", NULL},
            {"verbose", 'v', 0, G_OPTION_ARG_NONE, &config->verbose, "Enable verbose logging", NULL},
            {"once", 'o', 0, G_OPTION_ARG_NONE, &config->once, "Immediately trigger and run once", NULL},
            {NULL},
        };
    g_option_context_add_main_entries(context, entries, NULL);

    // parse command line arguments
    g_option_context_parse(context, &argc, &argv, &error);
    g_option_context_free(context);
    if (error)
    {
        g_warning("command line: %s", error->message);
        config_valid = FALSE;
    }
    g_clear_error(&error);

    // if config path not given use default
    if (!config_path)
        config_path = g_strdup(DEFAULT_CONFIG_PATH);

    // get the key file
    GKeyFile *key_file = g_key_file_new();
    g_key_file_set_list_separator(key_file, ',');
    g_key_file_load_from_file(key_file, config_path, G_KEY_FILE_NONE, &error);
    g_free(config_path);
    if (error && config_valid)
    {
        g_warning("config: Failed to load config %s", error->message);
        config_valid = FALSE;
    }
    g_clear_error(&error);

    // get app
    config->app = app_new_config(key_file);
    if (!config->app)
        config_valid = FALSE;

    // free key file
    g_key_file_free(key_file);

    // return
    if (!config_valid)
    {
        config_destroy(config);
        return NULL;
    }
    g_debug("config: Parse successful");
    return config;
}

// destroy and free a config
void config_destroy(Config *config)
{
    if (!config)
        return;

    app_destroy_config(config->app);

    g_free(config);
}