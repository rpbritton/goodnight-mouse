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

#include "tag_config.h"

#include "styler.h"

#define CONFIG_GROUP "tag"

// creates a tag config from a key file and default values
TagConfig *tag_new_config(GKeyFile *key_file)
{
    // create config
    TagConfig *config = g_new0(TagConfig, 1);
    gboolean config_valid = TRUE;

    GError *error = NULL;

    // create styler
    Styler *styler = styler_start();

    // get text_color
    gchar *text_color_string = g_key_file_get_string(key_file, CONFIG_GROUP,
                                                     "text_color", NULL);
    if (text_color_string)
    {
        GdkRGBA text_color;
        if (gdk_rgba_parse(&text_color, text_color_string))
        {
            gchar *prop = gdk_rgba_to_string(&text_color);
            styler_add(styler, TAG_CHARACTER_CSS_CLASS, "color", prop);
            g_free(prop);
        }
        else
        {
            g_warning("config: tag: text_color: Invalid color format '%s'", text_color_string);
            config_valid = FALSE;
        }
        g_free(text_color_string);
    }
    else
    {
        // default
        styler_add(styler, TAG_CHARACTER_CSS_CLASS, "color", "rgb(0, 255, 0)");
    }

    // get text_active_color
    gchar *text_active_color_string = g_key_file_get_string(key_file, CONFIG_GROUP,
                                                            "text_active_color", NULL);
    if (text_active_color_string)
    {
        GdkRGBA text_active_color;
        if (gdk_rgba_parse(&text_active_color, text_active_color_string))
        {
            gchar *prop = gdk_rgba_to_string(&text_active_color);
            styler_add(styler, TAG_CHARACTER_ACTIVE_CSS_CLASS, "color", prop);
            g_free(prop);
        }
        else
        {
            g_warning("config: tag: text_active_color: Invalid color format '%s'", text_active_color_string);
            config_valid = FALSE;
        }
        g_free(text_active_color_string);
    }
    else
    {
        // default
        styler_add(styler, TAG_CHARACTER_ACTIVE_CSS_CLASS, "color", "rgb(0, 0, 255)");
    }

    // get font_family
    gchar *font_family = g_key_file_get_string(key_file, CONFIG_GROUP,
                                               "font_family", NULL);
    if (font_family)
    {
        styler_add(styler, TAG_CHARACTER_CSS_CLASS, "font-family", font_family);
        g_free(font_family);
    }
    else
    {
        // default
        styler_add(styler, TAG_CHARACTER_CSS_CLASS, "font-family", "monospace");
    }

    // get font_size
    gint font_size = g_key_file_get_integer(key_file, CONFIG_GROUP,
                                            "font_size", &error);
    if (!error)
    {
        gchar *prop = g_strdup_printf("%dpx", font_size);
        styler_add(styler, TAG_CHARACTER_CSS_CLASS, "font-size", prop);
        g_free(prop);
    }
    else if (g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE))
    {
        g_warning("config: tag: font_size: Parse failed");
        config_valid = FALSE;
    }
    else
    {
        // default
        styler_add(styler, TAG_CHARACTER_CSS_CLASS, "font-size", "14px");
    }
    g_clear_error(&error);

    // get font_bold
    gboolean font_bold = g_key_file_get_boolean(key_file, CONFIG_GROUP,
                                                "font_bold", &error);
    if (!error)
    {
        styler_add(styler, TAG_CHARACTER_CSS_CLASS, "font-weight", (font_bold) ? "bold" : "normal");
    }
    else if (g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE))
    {
        g_warning("config: tag: font_bold: Parse failed");
        config_valid = FALSE;
    }
    else
    {
        // default
        styler_add(styler, TAG_CHARACTER_CSS_CLASS, "font-weight", "bold");
    }
    g_clear_error(&error);

    // get font_italic
    gboolean font_italic = g_key_file_get_boolean(key_file, CONFIG_GROUP,
                                                  "font_italic", &error);
    if (!error)
    {
        styler_add(styler, TAG_CHARACTER_CSS_CLASS, "font-style", (font_italic) ? "italic" : "normal");
    }
    else if (g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE))
    {
        g_warning("config: tag: font_italic: Parse failed");
        config_valid = FALSE;
    }
    else
    {
        // default
        styler_add(styler, TAG_CHARACTER_CSS_CLASS, "font-style", "normal");
    }
    g_clear_error(&error);

    // get color
    gchar *color_string = g_key_file_get_string(key_file, CONFIG_GROUP,
                                                "color", NULL);
    if (color_string)
    {
        GdkRGBA color;
        if (gdk_rgba_parse(&color, color_string))
        {
            gchar *prop = gdk_rgba_to_string(&color);
            styler_add(styler, TAG_LABEL_CSS_CLASS, "background-color", prop);
            g_free(prop);
        }
        else
        {
            g_warning("config: tag: color: Invalid color format '%s'", color_string);
            config_valid = FALSE;
        }
        g_free(color_string);
    }
    else
    {
        // default
        styler_add(styler, TAG_LABEL_CSS_CLASS, "background-color", "rgb(0, 0, 0)");
    }

    // get padding
    gsize num_padding;
    gint *padding = g_key_file_get_integer_list(key_file, CONFIG_GROUP,
                                                "padding", &num_padding, &error);
    if (padding)
    {
        gchar *prop = NULL;
        switch (num_padding)
        {
        case 1:
            prop = g_strdup_printf("%dpx", padding[0]);
            break;
        case 2:
            prop = g_strdup_printf("%dpx %dpx", padding[0], padding[1]);
            break;
        case 3:
            prop = g_strdup_printf("%dpx %dpx %dpx", padding[0], padding[1], padding[2]);
            break;
        case 4:
            prop = g_strdup_printf("%dpx %dpx %dpx %dpx", padding[0], padding[1], padding[2], padding[3]);
            break;
        default:
            g_warning("config: tag: padding: Cannot handle '%lu' padding values", num_padding);
            config_valid = FALSE;
            break;
        }
        g_free(padding);

        if (prop)
        {
            styler_add(styler, TAG_LABEL_CSS_CLASS, "padding", prop);
            g_free(prop);
        }
    }
    else if (g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE))
    {
        g_warning("config: tag: padding: Parse failed");
        config_valid = FALSE;
    }
    else
    {
        // default
        styler_add(styler, TAG_LABEL_CSS_CLASS, "padding", "1px 3px");
    }
    g_clear_error(&error);

    // get alignment
    gint alignment = g_key_file_get_integer(key_file, CONFIG_GROUP,
                                            "alignment", &error);
    if (!error)
    {
        switch (alignment)
        {
        case 1:
            config->alignment_horizontal = GTK_ALIGN_START;
            config->alignment_vertical = GTK_ALIGN_START;
            break;
        case 2:
            config->alignment_horizontal = GTK_ALIGN_CENTER;
            config->alignment_vertical = GTK_ALIGN_START;
            break;
        case 3:
            config->alignment_horizontal = GTK_ALIGN_END;
            config->alignment_vertical = GTK_ALIGN_START;
            break;
        case 4:
            config->alignment_horizontal = GTK_ALIGN_START;
            config->alignment_vertical = GTK_ALIGN_CENTER;
            break;
        case 5:
            config->alignment_horizontal = GTK_ALIGN_CENTER;
            config->alignment_vertical = GTK_ALIGN_CENTER;
            break;
        case 6:
            config->alignment_horizontal = GTK_ALIGN_END;
            config->alignment_vertical = GTK_ALIGN_CENTER;
            break;
        case 7:
            config->alignment_horizontal = GTK_ALIGN_START;
            config->alignment_vertical = GTK_ALIGN_END;
            break;
        case 8:
            config->alignment_horizontal = GTK_ALIGN_CENTER;
            config->alignment_vertical = GTK_ALIGN_END;
            break;
        case 9:
            config->alignment_horizontal = GTK_ALIGN_END;
            config->alignment_vertical = GTK_ALIGN_END;
            break;
        default:
            g_warning("config: tag: alignment: Index out of range '%d'", alignment);
            config_valid = FALSE;
            break;
        }
    }
    else if (g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE))
    {
        g_warning("config: tag: alignment: Parse failed");
        config_valid = FALSE;
    }
    else
    {
        // default
        config->alignment_horizontal = GTK_ALIGN_START;
        config->alignment_vertical = GTK_ALIGN_CENTER;
    }
    g_clear_error(&error);

    // get border_style
    styler_add(styler, TAG_LABEL_CSS_CLASS, "border-style", "solid");

    // get border_color
    gchar *border_color_string = g_key_file_get_string(key_file, CONFIG_GROUP,
                                                       "border_color", NULL);
    if (border_color_string)
    {
        GdkRGBA border_color;
        if (gdk_rgba_parse(&border_color, border_color_string))
        {
            gchar *prop = gdk_rgba_to_string(&border_color);
            styler_add(styler, TAG_LABEL_CSS_CLASS, "border-color", prop);
            g_free(prop);
        }
        else
        {
            g_warning("config: tag: border_color: Invalid color format '%s'", border_color_string);
            config_valid = FALSE;
        }
        g_free(border_color_string);
    }
    else
    {
        // default
        styler_add(styler, TAG_LABEL_CSS_CLASS, "border-color", "rgb(255, 255, 255)");
    }

    // get border_width
    gint border_width = g_key_file_get_integer(key_file, CONFIG_GROUP,
                                               "border_width", &error);
    if (!error)
    {
        gchar *prop = g_strdup_printf("%dpx", border_width);
        styler_add(styler, TAG_LABEL_CSS_CLASS, "border-width", prop);
        g_free(prop);
    }
    else if (g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE))
    {
        g_warning("config: tag: border_width: Parse failed");
        config_valid = FALSE;
    }
    else
    {
        // default
        styler_add(styler, TAG_LABEL_CSS_CLASS, "border-width", "1px");
    }
    g_clear_error(&error);

    // get border_radius
    gint border_radius = g_key_file_get_integer(key_file, CONFIG_GROUP,
                                                "border_radius", &error);
    if (!error)
    {
        gchar *prop = g_strdup_printf("%dpx", border_radius);
        styler_add(styler, TAG_LABEL_CSS_CLASS, "border-radius", prop);
        g_free(prop);
    }
    else if (g_error_matches(error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_INVALID_VALUE))
    {
        g_warning("config: tag: border_radius: Parse failed");
        config_valid = FALSE;
    }
    else
    {
        // default
        styler_add(styler, TAG_LABEL_CSS_CLASS, "border-radius", "3px");
    }
    g_clear_error(&error);

    // finish styler
    config->styling = GTK_STYLE_PROVIDER(styler_finish(styler));

    // return
    if (!config_valid)
    {
        tag_destroy_config(config);
        return NULL;
    }
    return config;
}

// destroys and frees a tag config
void tag_destroy_config(TagConfig *config)
{
    if (!config)
        return;

    g_object_unref(config->styling);

    g_free(config);
}
