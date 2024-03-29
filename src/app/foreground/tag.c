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

static void tag_reposition(Tag *tag);
static void tag_generate_label(Tag *tag);
static void tag_destroy_label(Tag *tag);
static void tag_show_label(Tag *tag);

// creates a new tag
Tag *tag_new(TagConfig *config)
{
    Tag *tag = g_new(Tag, 1);

    // init members
    tag->code = NULL;
    tag->match_index = 0;

    tag->accessible = NULL;

    tag->shifted = FALSE;

    // set parent window
    tag->parent = NULL;
    tag->window_x = 0;
    tag->window_y = 0;

    // create the label
    tag->label = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(tag->label), TAG_LABEL_CSS_CLASS);
    gtk_widget_set_hexpand(tag->label, FALSE);
    gtk_widget_set_vexpand(tag->label, FALSE);

    // create the label wrapper
    tag->wrapper = g_object_ref(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
    gtk_style_context_add_class(gtk_widget_get_style_context(tag->wrapper), TAG_CONTAINER_CSS_CLASS);
    gtk_box_pack_start(GTK_BOX(tag->wrapper), tag->label, TRUE, TRUE, 0);
    gtk_widget_set_no_show_all(tag->wrapper, TRUE);

    // init the gtk labels
    tag->characters = NULL;

    // set styling
    gtk_style_context_add_provider_for_screen(gtk_widget_get_screen(tag->wrapper),
                                              config->styling,
                                              GTK_STYLE_PROVIDER_PRIORITY_SETTINGS);

    // set alignment
    gtk_widget_set_halign(tag->label, config->alignment_horizontal);
    gtk_widget_set_valign(tag->label, config->alignment_vertical);

    return tag;
}

// destroys and frees a tag
void tag_destroy(Tag *tag)
{
    // hide tag
    tag_hide(tag);

    // unset code
    tag_unset_code(tag);

    // unset configuration
    tag_unset_accessible(tag);

    // destroy the label
    tag_destroy_label(tag);

    // destroy all gtk elements
    gtk_widget_destroy(tag->wrapper);
    g_object_unref(tag->wrapper);

    g_free(tag);
}

// sets a tag to follow an accessible
void tag_set_accessible(Tag *tag, AtspiAccessible *accessible)
{
    // unset last accessible
    tag_unset_accessible(tag);

    // set accessible
    tag->accessible = g_object_ref(accessible);

    // reposition if shown
    if (tag->parent)
        tag_reposition(tag);
}

// stops a tag from following an accessible
void tag_unset_accessible(Tag *tag)
{
    // do nothing if not set
    if (!tag->accessible)
        return;

    // unset accessible
    g_object_unref(tag->accessible);
    tag->accessible = NULL;
}

// shiftes a tag to show upper or lower case
void tag_shifted(Tag *tag, gboolean shifted)
{
    // do nothing if not changed
    if (shifted == tag->shifted)
        return;

    // set shift
    tag->shifted = shifted;

    // regenerate label
    tag_generate_label(tag);
}

// add and show a tag in a gtk layout
void tag_show(Tag *tag, GtkLayout *parent, gint window_x, gint window_y)
{
    // return if invalid parent
    if (!parent)
        return;

    // add new parent
    if (parent != tag->parent)
    {
        // hide if already showing
        tag_hide(tag);

        // add the new parent
        tag->parent = g_object_ref(parent);

        // generate the label
        tag_generate_label(tag);

        // show the wrapper
        gtk_widget_show(tag->wrapper);
    }

    // set window coordinates
    tag->window_x = window_x;
    tag->window_y = window_y;

    // reposition the tag
    tag_reposition(tag);
}

// removes a tag from its parent
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

// repositions a tag over its accessible
void tag_reposition(Tag *tag)
{
    // stop if no accessible
    if (!tag->accessible)
        return;

    // get accessible position
    AtspiComponent *component = atspi_accessible_get_component_iface(tag->accessible);
    AtspiRect *rect = atspi_component_get_extents(component, ATSPI_COORD_TYPE_SCREEN, NULL);

    // offset with window coordinates
    rect->x -= tag->window_x;
    rect->y -= tag->window_y;

    // put/move location in parent if coordinates are valid
    if (rect->x >= 0 && rect->y >= 0)
    {
        if (gtk_widget_get_parent(tag->wrapper) == GTK_WIDGET(tag->parent))
            gtk_layout_move(tag->parent, tag->wrapper, rect->x, rect->y);
        else
            gtk_layout_put(tag->parent, tag->wrapper, rect->x, rect->y);
    }

    // set wrapper to cover accessible
    if (rect->width > 0 && rect->height > 0)
        gtk_widget_set_size_request(tag->wrapper, rect->width, rect->height);

    // free
    g_object_unref(component);
    g_free(rect);
}

// sets a tag's code
void tag_set_code(Tag *tag, GArray *code)
{
    // set code
    tag_unset_code(tag);
    tag->code = g_array_ref(code);

    // generate label if showing
    if (tag->parent)
        tag_generate_label(tag);
}

// returns a tag's code
GArray *tag_get_code(Tag *tag)
{
    return g_array_ref(tag->code);
}

// unsets a tag's code
void tag_unset_code(Tag *tag)
{
    if (!tag->code)
        return;

    // reset code
    g_array_unref(tag->code);
    tag->code = NULL;
    tag->match_index = 0;
}

// applies a code to a tag, marking some characters as active or hiding the tag
// if the codes mismatch
gboolean tag_apply_code(Tag *tag, GArray *code)
{
    gint match_index = 0;

    // compare code
    for (gint index = 0; index < code->len; index++)
    {
        // code is too long; invalid
        if (index == tag->code->len)
        {
            match_index = -1;
            break;
        }

        // break if character doesn't match
        if (g_array_index(code, guint, index) != g_array_index(tag->code, guint, index))
        {
            match_index = -1;
            break;
        }

        // so far so good
        match_index++;
    }

    // set match index
    tag->match_index = match_index;
    tag_show_label(tag);

    // return whether valid
    return tag->match_index > -1;
}

// returns if tag perfectly matches the last applied code
gboolean tag_matches_code(Tag *tag)
{
    return tag->match_index == tag->code->len;
}

// generates the tag label from the code
static void tag_generate_label(Tag *tag)
{
    // remove old label
    tag_destroy_label(tag);

    // create space to hold label references
    tag->characters = g_array_sized_new(FALSE, FALSE, sizeof(GtkWidget *), tag->code->len);

    // create labels
    for (gint index = 0; index < tag->code->len; index++)
    {
        guint key = g_array_index(tag->code, guint, index);
        if (tag->shifted)
            key = gdk_keyval_to_upper(key);
        else
            key = gdk_keyval_to_lower(key);

        gunichar unicode = gdk_keyval_to_unicode(key);
        gchar *unicode_str = g_ucs4_to_utf8(&unicode, 1, NULL, NULL, NULL);

        GtkWidget *character = gtk_label_new(unicode_str);
        g_free(unicode_str);

        gtk_style_context_add_class(gtk_widget_get_style_context(character), TAG_CHARACTER_CSS_CLASS);

        gtk_container_add(GTK_CONTAINER(tag->label), character);
        g_array_append_val(tag->characters, character);
    }

    // show the label
    tag_show_label(tag);
}

// destroys the current tag label
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

// shows the tag label and sets active character css class
static void tag_show_label(Tag *tag)
{
    // update label character css classes
    if (tag->characters)
    {
        for (gint index = 0; index < tag->characters->len; index++)
        {
            GtkStyleContext *context = gtk_widget_get_style_context(g_array_index(tag->characters,
                                                                                  GtkWidget *,
                                                                                  index));

            if (index < tag->match_index)
                gtk_style_context_add_class(context, TAG_CHARACTER_ACTIVE_CSS_CLASS);
            else
                gtk_style_context_remove_class(context, TAG_CHARACTER_ACTIVE_CSS_CLASS);
        }
    }

    // hide or show the label
    if (tag->match_index > -1)
        gtk_widget_show_all(tag->label);
    else
        gtk_widget_hide(tag->label);
}
