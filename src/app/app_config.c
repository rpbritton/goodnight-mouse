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

#include "app_config.h"

// create an app config using the key file and default values
AppConfig *app_new_config(GKeyFile *key_file)
{
    // create config
    AppConfig *config = g_new0(AppConfig, 1);
    gboolean config_valid = TRUE;

    // get background
    config->background = background_new_config(key_file);
    if (!config->background)
        config_valid = FALSE;

    // get foreground
    config->foreground = foreground_new_config(key_file);
    if (!config->foreground)
        config_valid = FALSE;

    // return
    if (!config_valid)
    {
        app_destroy_config(config);
        return NULL;
    }
    return config;
}

// destroy and free the app config
void app_destroy_config(AppConfig *config)
{
    if (!config)
        return;

    background_destroy_config(config->background);
    foreground_destroy_config(config->foreground);

    g_free(config);
}