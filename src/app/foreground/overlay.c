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

#include "overlay.h"

static void remove_input(GtkWidget *overlay, gpointer data);

static void overlay_refresh(Overlay *overlay);
static void overlay_reposition(Overlay *overlay);

Overlay *overlay_new(OverlayConfig *config)
{
    Overlay *overlay = g_new(Overlay, 1);

    overlay->window = NULL;
    overlay->controls = g_hash_table_new(NULL, NULL);

    // create overlay
    overlay->overlay = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_title(GTK_WINDOW(overlay->overlay), OVERLAY_WINDOW_TITLE);

    // set css styling
    GtkStyleContext *style_context = gtk_widget_get_style_context(overlay->overlay);
    gtk_style_context_add_class(style_context, OVERLAY_CSS_CLASS);
    gtk_style_context_add_provider(style_context, config->styling, GTK_STYLE_PROVIDER_PRIORITY_SETTINGS);
    // allow window transparency
    gtk_widget_set_visual(overlay->overlay, gdk_screen_get_rgba_visual(gtk_widget_get_screen(overlay->overlay)));

    // remove overlay interactivity
    gtk_window_set_accept_focus(GTK_WINDOW(overlay->overlay), FALSE);
    gtk_widget_set_sensitive(overlay->overlay, FALSE);
    g_signal_connect(G_OBJECT(overlay->overlay), "draw", G_CALLBACK(remove_input), NULL);

    // create container
    overlay->container = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(overlay->overlay), overlay->container);

    return overlay;
}

void overlay_destroy(Overlay *overlay)
{
    // hide if showing
    if (!overlay->window)
        overlay_hide(overlay);

    // remove control references
    g_hash_table_unref(overlay->controls);

    // free overlay window
    gtk_widget_destroy(overlay->overlay);

    g_free(overlay);
}

void overlay_show(Overlay *overlay, AtspiAccessible *window)
{
    if (!window)
        return;

    // set the new window
    if (overlay->window)
        g_object_unref(overlay->window);
    overlay->window = g_object_ref(window);

    // show all controls
    GHashTableIter iter;
    gpointer control_ptr, null_ptr;
    g_hash_table_iter_init(&iter, overlay->controls);
    while (g_hash_table_iter_next(&iter, &control_ptr, &null_ptr))
        control_show(control_ptr, GTK_FIXED(overlay->container));

    // reposition and show the window
    overlay_reposition(overlay);
    gtk_widget_show_all(overlay->overlay);

    // todo: add time interval call to overlay_refresh();
}

void overlay_hide(Overlay *overlay)
{
    if (!overlay->window)
        return;

    // remove the old window
    g_object_unref(overlay->window);
    overlay->window = NULL;

    // hide all controls
    GHashTableIter iter;
    gpointer control_ptr, null_ptr;
    g_hash_table_iter_init(&iter, overlay->controls);
    while (g_hash_table_iter_next(&iter, &control_ptr, &null_ptr))
        control_hide(control_ptr);

    // hide the overlay
    gtk_widget_hide(overlay->overlay);
}

void overlay_add(Overlay *overlay, Control *control)
{
    // add control reference
    gboolean added = g_hash_table_add(overlay->controls, control);
    if (!added)
        return;

    // show control if overlay is shown
    if (overlay->window)
        control_show(control, GTK_FIXED(overlay->container));
}

void overlay_remove(Overlay *overlay, Control *control)
{
    // remove control reference
    gboolean removed = g_hash_table_remove(overlay->controls, control);
    if (!removed)
        return;

    // hide control if overlay is shown
    if (overlay->window)
        control_hide(control);
}

static void remove_input(GtkWidget *overlay, gpointer data)
{
    cairo_rectangle_int_t rectangle = {
        .x = 0,
        .y = 0,
        .width = 0,
        .height = 0,
    };
    cairo_region_t *region = cairo_region_create_rectangle(&rectangle);
    gdk_window_input_shape_combine_region(gtk_widget_get_window(overlay), region, 0, 0);
    cairo_region_destroy(region);
}

static void overlay_refresh(Overlay *overlay)
{
    // do nothing if not shown
    if (!overlay->window)
        return;

    // reposition the overlay
    overlay_reposition(overlay);

    // reposition the controls
    GHashTableIter iter;
    gpointer control_ptr, null_ptr;
    g_hash_table_iter_init(&iter, overlay->controls);
    while (g_hash_table_iter_next(&iter, &control_ptr, &null_ptr))
        control_reposition(control_ptr);
}

static void overlay_reposition(Overlay *overlay)
{
    AtspiComponent *component = atspi_accessible_get_component_iface(overlay->window);
    AtspiRect *rect = atspi_component_get_extents(component, ATSPI_COORD_TYPE_SCREEN, NULL);

    gtk_window_move(GTK_WINDOW(overlay->overlay), rect->x, rect->y);
    gtk_window_resize(GTK_WINDOW(overlay->overlay), rect->width, rect->height);

    g_object_unref(component);
    g_free(rect);
}