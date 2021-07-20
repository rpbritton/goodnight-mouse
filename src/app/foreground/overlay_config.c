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

#include "overlay_config.h"

#include "styler.h"

#define CONFIG_GROUP "overlay"

OverlayConfig *overlay_new_config(GKeyFile *key_file)
{
    // create config
    OverlayConfig *config = g_new0(OverlayConfig, 1);
    gboolean config_valid = TRUE;

    // create styler
    Styler *styler = styler_start();

    // parse color
    gchar *color_string = g_key_file_get_string(key_file, CONFIG_GROUP,
                                                "color", NULL);
    if (color_string)
    {
        GdkRGBA color;
        if (gdk_rgba_parse(&color, color_string))
        {
            gchar *prop = gdk_rgba_to_string(&color);
            styler_add(styler, OVERLAY_CSS_CLASS, "background-color", prop);
            g_free(prop);
        }
        else
        {
            g_warning("config: overlay: color: Invalid color format '%s'", color_string);
            config_valid = FALSE;
        }
        g_free(color_string);
    }
    else
    {
        // default
        styler_add(styler, OVERLAY_CSS_CLASS, "background-color", "rgba(255, 0, 0, 0.05)");
    }

    // finish styler
    config->styling = GTK_STYLE_PROVIDER(styler_finish(styler));

    // return
    if (!config_valid)
    {
        overlay_destroy_config(config);
        return NULL;
    }
    return config;
}

void overlay_destroy_config(OverlayConfig *config)
{
    if (!config)
        return;

    g_object_unref(config->styling);

    g_free(config);
}