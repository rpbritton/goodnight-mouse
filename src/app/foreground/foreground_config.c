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

#include "foreground_config.h"

ForegroundConfig *foreground_new_config(GKeyFile *key_file)
{
    // create config
    ForegroundConfig *config = g_new0(ForegroundConfig, 1);
    gboolean config_valid = TRUE;

    // get overlay
    config->overlay = overlay_new_config(key_file);
    if (!config->overlay)
        config_valid = FALSE;

    // get codes
    config->codes = codes_new_config(key_file);
    if (!config->codes)
        config_valid = FALSE;

    // return
    if (!config_valid)
    {
        foreground_destroy_config(config);
        return NULL;
    }
    return config;
}

void foreground_destroy_config(ForegroundConfig *config)
{
    if (!config)
        return;

    overlay_destroy_config(config->overlay);
    codes_destroy_config(config->codes);

    g_free(config);
}