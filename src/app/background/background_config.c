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

#include "background_config.h"

#include <gdk/gdk.h>

#define CONFIG_GROUP "background"

// creates a background config using a key file and default values
BackgroundConfig *background_new_config(GKeyFile *key_file)
{
    // create config
    BackgroundConfig *config = g_new0(BackgroundConfig, 1);
    gboolean config_valid = TRUE;

    // get key
    gchar *key_string = g_key_file_get_string(key_file, CONFIG_GROUP,
                                              "key", NULL);
    if (key_string)
    {
        // parse string
        config->key = gdk_keyval_from_name(key_string);
        if (config->key == GDK_KEY_VoidSymbol)
        {
            g_warning("config: background: key: Unknown '%s'", key_string);
            config_valid = FALSE;
        }
        g_free(key_string);
    }
    else
    {
        // default
        config->key = GDK_KEY_g;
    }

    // get modifiers
    gsize num_modifiers;
    gchar **modifier_strings = g_key_file_get_string_list(key_file, CONFIG_GROUP,
                                                          "modifiers", &num_modifiers,
                                                          NULL);
    if (modifier_strings)
    {
        // parse modifiers
        config->modifiers = 0;
        for (gint index = 0; index < num_modifiers; index++)
        {
            gchar *modifier_string = modifier_strings[index];
            if (g_ascii_strcasecmp(modifier_string, "shift") == 0)
                config->modifiers |= GDK_SHIFT_MASK;
            else if (g_ascii_strcasecmp(modifier_string, "control") == 0)
                config->modifiers |= GDK_CONTROL_MASK;
            else if (g_ascii_strcasecmp(modifier_string, "super") == 0)
                config->modifiers |= GDK_SUPER_MASK;
            else if (g_ascii_strcasecmp(modifier_string, "hyper") == 0)
                config->modifiers |= GDK_HYPER_MASK;
            else if (g_ascii_strcasecmp(modifier_string, "alt") == 0)
                config->modifiers |= GDK_META_MASK;
            else
            {
                g_warning("config: background: modifiers: Unknown modifier '%s'", modifier_string);
                config_valid = FALSE;
            }
        }
        g_strfreev(modifier_strings);

        // ensure one modifier
        if (config->modifiers == 0)
        {
            g_warning("config: background: modifiers: At least one modifier is required");
            config_valid = FALSE;
        }
    }
    else
    {
        // default
        config->modifiers = GDK_SUPER_MASK;
    }

    // use upper case key if shift is set
    if (config->modifiers & GDK_SHIFT_MASK)
        config->key = gdk_keyval_to_upper(config->key);
    else
        config->key = gdk_keyval_to_lower(config->key);

    // return
    if (!config_valid)
    {
        background_destroy_config(config);
        return NULL;
    }
    return config;
}

// destroys and frees a background config
void background_destroy_config(BackgroundConfig *config)
{
    if (!config)
        return;

    g_free(config);
}