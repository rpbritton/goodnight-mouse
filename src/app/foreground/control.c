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

#include "control.h"

#include "identify.h"
#include "execution.h"

static void control_reveal(Control *control);
static void control_conceal(Control *control);

Control *control_new(ControlType type, AtspiAccessible *accessible)
{
    Control *control = g_new(Control, 1);

    control->type = type;
    control->accessible = g_object_ref(accessible);

    control->code = NULL;

    control->parent = NULL;

    // create the tag
    control->container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    control->tag = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(control->tag), CONTROL_CSS_CLASS_TAG);
    gtk_widget_set_halign(control->tag, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(control->container), control->tag);

    control->labels = NULL;

    return control;
}

void control_destroy(Control *control)
{
    g_object_unref(control->accessible);

    // remove label
    control_unlabel(control);

    // hide if showing
    control_hide(control);

    // destroy tag
    gtk_widget_destroy(GTK_WIDGET(control->container));

    g_free(control);
}

void control_execute(Control *control)
{
    switch (control->type)
    {
    case CONTROL_TYPE_PRESS:
        control_execution_press(control);
        break;
    case CONTROL_TYPE_FOCUS:
        control_execution_focus(control);
        break;
    default:
        break;
    }
}

void control_label(Control *control, GArray *code)
{
    // remove old label
    control_unlabel(control);

    // set new label
    control->code = g_array_ref(code);

    // re-reveal
    if (control->labels)
        control_reveal(control);
}

void control_unlabel(Control *control)
{
    // do nothing if no label
    if (!control->code)
        return;

    // remove saved code
    g_array_unref(control->code);
    control->code = NULL;

    // free any labelling
    control_conceal(control);
}

void control_show(Control *control, GtkFixed *parent)
{
    // hide if showing
    if (control->parent)
        control_hide(control);

    // add new parent
    control->parent = g_object_ref(parent);
    control_reposition(control);

    // generate the label
    control_reveal(control);

    // show the container
    gtk_widget_show_all(control->container);
}

void control_hide(Control *control)
{
    // return if not showing
    if (!control->parent)
        return;

    // remove the tag from the parent
    gtk_widget_unparent(control->container);

    // remove parent reference
    g_object_unref(control->parent);
    control->parent = NULL;
}

void control_reposition(Control *control)
{
    AtspiComponent *component = atspi_accessible_get_component_iface(control->accessible);
    AtspiRect *rect = atspi_component_get_extents(component, ATSPI_COORD_TYPE_WINDOW, NULL);

    if (gtk_widget_get_parent(control->container) == GTK_WIDGET(control->parent))
        gtk_fixed_move(control->parent, control->container, rect->x, rect->y);
    else
        gtk_fixed_put(control->parent, control->container, rect->x, rect->y);

    gtk_widget_set_size_request(control->container, rect->width, rect->height);

    g_object_unref(component);
    g_free(rect);
}

static void control_reveal(Control *control)
{
    // remove old label
    if (control->labels)
        control_conceal(control);

    // create space to hold label references
    control->labels = g_array_sized_new(FALSE, FALSE, sizeof(GtkWidget *), control->code->len);

    // create labels
    for (gint index = 0; index < control->code->len; index++)
    {
        gunichar unicode = gdk_keyval_to_unicode(g_array_index(control->code, guint, index));
        gchar *unicode_str = g_ucs4_to_utf8(&unicode, 1, NULL, NULL, NULL);

        GtkWidget *label = gtk_label_new(unicode_str);
        g_free(unicode_str);

        gtk_style_context_add_class(gtk_widget_get_style_context(label), CONTROL_CSS_CLASS_LABEL);

        gtk_container_add(GTK_CONTAINER(control->tag), label);
        gtk_widget_show_all(control->container);

        g_array_append_val(control->labels, label);
    }
}

static void control_conceal(Control *control)
{
    // do nothing if not revealed
    if (!control->labels)
        return;

    // remove labels
    for (gint index = 0; index < control->labels->len; index++)
        gtk_widget_destroy(g_array_index(control->labels, GtkWidget *, index));

    // remove labels reference
    g_array_unref(control->labels);
    control->labels = NULL;
}