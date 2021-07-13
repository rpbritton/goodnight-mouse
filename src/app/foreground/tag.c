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

#include "tag.h"

static void tag_generate_label(Tag *tag);
static void tag_destroy_label(Tag *tag);

Tag *tag_new()
{
    Tag *tag = g_new(Tag, 1);

    // init members
    tag->code = NULL;

    tag->parent = NULL;

    // create the label
    tag->label = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(tag->label), TAG_LABEL_CSS_CLASS);
    gtk_widget_set_hexpand(tag->label, FALSE);
    gtk_widget_set_vexpand(tag->label, FALSE);

    // create the label wrapper
    tag->wrapper = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(tag->wrapper), TAG_CONTAINER_CSS_CLASS);

    gtk_container_add(GTK_CONTAINER(tag->wrapper), tag->label);

    // init the gtk labels
    tag->characters = NULL;

    // set default config
    tag->accessible = NULL;
    tag->styling = NULL;
    tag_unset_config(tag);

    return tag;
}

void tag_destroy(Tag *tag)
{
    // hide tag
    tag_hide(tag);

    // unset code
    tag_unset_code(tag);

    // unset configuration
    tag_unset_config(tag);

    // destroy the label
    tag_destroy_label(tag);

    // destroy all gtk elements
    gtk_widget_destroy(tag->wrapper);

    g_free(tag);
}

void tag_set_config(Tag *tag, TagConfig *config)
{
    // unsef old config
    tag_unset_config(tag);

    // set accessible
    tag->accessible = g_object_ref(config->accessible);
    // reposition if shown
    if (tag->parent)
        tag_reposition(tag);

    // set styling
    tag->styling = g_object_ref(config->styling);
    gtk_style_context_add_provider_for_screen(gtk_widget_get_screen(tag->wrapper),
                                              tag->styling, GTK_STYLE_PROVIDER_PRIORITY_SETTINGS);

    // set alignment
    gtk_widget_set_halign(tag->label, config->alignment_horizontal);
    gtk_widget_set_valign(tag->label, config->alignment_vertical);
}

void tag_unset_config(Tag *tag)
{
    // unset accessible
    if (tag->accessible)
    {
        g_object_unref(tag->accessible);
        tag->accessible = NULL;
    }

    // unset styling
    if (tag->styling)
    {
        gtk_style_context_remove_provider_for_screen(gtk_widget_get_screen(tag->wrapper),
                                                     tag->styling);
        g_object_unref(tag->styling);
        tag->styling = NULL;
    }

    // set default alignment
    gtk_widget_set_halign(tag->label, GTK_ALIGN_START);
    gtk_widget_set_valign(tag->label, GTK_ALIGN_START);
}

void tag_show(Tag *tag, GtkFixed *parent)
{
    // return if invalid parent
    if (!parent || tag->parent == parent)
        return;

    // hide if already showing
    if (tag->parent)
        tag_hide(tag);

    // add the new parent
    tag->parent = g_object_ref(parent);
    tag_reposition(tag);

    // generate the label
    tag_generate_label(tag);

    // show the wrapper
    gtk_widget_show_all(tag->wrapper);
}

void tag_hide(Tag *tag)
{
    // return already hidden
    if (!tag->parent)
        return;

    // remove the tag from the parent
    gtk_container_remove(GTK_CONTAINER(tag->parent), tag->wrapper);

    // remove parent reference
    g_object_unref(tag->parent);
    tag->parent = NULL;
}

void tag_reposition(Tag *tag)
{
    // stop if no accessible
    if (!tag->accessible)
        return;

    // todo: check if accessible is visible

    // get accessible position
    AtspiComponent *component = atspi_accessible_get_component_iface(tag->accessible);
    AtspiRect *rect = atspi_component_get_extents(component, ATSPI_COORD_TYPE_WINDOW, NULL);

    // put/move accessible in parent
    if (gtk_widget_get_parent(tag->wrapper) == GTK_WIDGET(tag->parent))
        gtk_fixed_move(tag->parent, tag->wrapper, rect->x, rect->y);
    else
        gtk_fixed_put(tag->parent, tag->wrapper, rect->x, rect->y);

    // set wrapper to cover accessible
    gtk_widget_set_size_request(tag->wrapper, rect->width, rect->height);

    // free
    g_object_unref(component);
    g_free(rect);
}

void tag_set_code(Tag *tag, GArray *code)
{
    // set code
    tag_unset_code(tag);
    tag->code = g_array_ref(code);

    // generate label if showing
    if (tag->parent)
        tag_generate_label(tag);
}

GArray *tag_get_code(Tag *tag)
{
    return g_array_ref(tag->code);
}

void tag_unset_code(Tag *tag)
{
    if (!tag->code)
        return;

    // reset code
    g_array_unref(tag->code);
    tag->code = NULL;
}

gboolean tag_match_code(Tag *tag, GArray *code)
{
    // todo
    return FALSE;
}

static void tag_generate_label(Tag *tag)
{
    // remove old label
    tag_destroy_label(tag);

    // create space to hold label references
    tag->characters = g_array_sized_new(FALSE, FALSE, sizeof(GtkWidget *), tag->code->len);

    // create labels
    for (gint index = 0; index < tag->code->len; index++)
    {
        gunichar unicode = gdk_keyval_to_unicode(g_array_index(tag->code, guint, index));
        gchar *unicode_str = g_ucs4_to_utf8(&unicode, 1, NULL, NULL, NULL);

        GtkWidget *character = gtk_label_new(unicode_str);
        g_free(unicode_str);

        gtk_style_context_add_class(gtk_widget_get_style_context(character), TAG_CHARACTER_CSS_CLASS);

        gtk_container_add(GTK_CONTAINER(tag->label), character);
        g_array_append_val(tag->characters, character);
    }

    // show new label if shown
    if (tag->parent)
        gtk_widget_show_all(tag->wrapper);
}

static void tag_destroy_label(Tag *tag)
{
    // do nothing if no label exists
    if (!tag->characters)
        return;

    // remove labels
    for (gint index = 0; index < tag->characters->len; index++)
        gtk_widget_destroy(g_array_index(tag->characters, GtkWidget *, index));

    // remove labels reference
    g_array_unref(tag->characters);
    tag->characters = NULL;
}