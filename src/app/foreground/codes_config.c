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

#include "codes_config.h"

#include <gdk/gdk.h>

#define CONFIG_GROUP "codes"

CodesConfig *codes_new_config(GKeyFile *key_file)
{
    // create config
    CodesConfig *config = g_new0(CodesConfig, 1);
    gboolean config_valid = TRUE;

    // get keys
    gsize num_keys;
    gchar **key_strings = g_key_file_get_string_list(key_file, CONFIG_GROUP,
                                                     "keys", &num_keys, NULL);
    if (key_strings)
    {
        config->keys = g_array_sized_new(FALSE, FALSE, sizeof(guint), num_keys);
        for (gint index = 0; index < num_keys; index++)
        {
            guint key = gdk_keyval_from_name(key_strings[index]);
            if (key == GDK_KEY_VoidSymbol)
            {
                g_warning("config: codes: keys: Key not found '%s'", key_strings[index]);
                config_valid = FALSE;
            }
            guint lower_case_key = gdk_keyval_to_lower(key);
            g_array_append_val(config->keys, lower_case_key);
        }
        g_strfreev(key_strings);
    }
    else
    {
        // default
        config->keys = g_array_new(FALSE, FALSE, sizeof(guint));
        guint keys[] = {GDK_KEY_a, GDK_KEY_b, GDK_KEY_c};
        g_array_append_vals(config->keys, keys, 3);
    }

    // get consecutive_keys
    GError *error = NULL;
    config->consecutive_keys = g_key_file_get_boolean(key_file, CONFIG_GROUP,
                                                      "consecutive_keys", &error);
    if (g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE))
    {
        g_warning("config: codes: consecutive_keys: Parse failed");
        config_valid = FALSE;
    }
    else if (error != NULL)
    {
        // default
        config->consecutive_keys = FALSE;
    }
    g_clear_error(&error);

    // get tag
    config->tag = tag_new_config(key_file);
    if (!config->tag)
        config_valid = FALSE;

    // return
    if (!config_valid)
    {
        codes_destroy_config(config);
        return NULL;
    }
    return config;
}

void codes_destroy_config(CodesConfig *config)
{
    if (!config)
        return;

    g_free(config);
}