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

#define OVERLAY_REFRESH_INTERVAL 250

static void remove_input(GtkWidget *overlay, gpointer data);

static void overlay_refresh(Overlay *overlay);
static void overlay_reposition(Overlay *overlay);
static gboolean overlay_refresh_loop(gpointer overlay_ptr);

// creates a new overlay from the config
Overlay *overlay_new(OverlayConfig *config)
{
    Overlay *overlay = g_new(Overlay, 1);

    overlay->window = NULL;
    overlay->tags = g_hash_table_new(NULL, NULL);
    overlay->shifted = FALSE;

    // create overlay
    overlay->overlay = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_title(GTK_WINDOW(overlay->overlay), OVERLAY_WINDOW_TITLE);

    // set css styling
    gtk_style_context_add_class(gtk_widget_get_style_context(overlay->overlay), OVERLAY_CSS_CLASS);
    gtk_style_context_add_provider_for_screen(gtk_widget_get_screen(overlay->overlay),
                                              config->styling, GTK_STYLE_PROVIDER_PRIORITY_SETTINGS);
    // allow window transparency
    gtk_widget_set_visual(overlay->overlay, gdk_screen_get_rgba_visual(gtk_widget_get_screen(overlay->overlay)));

    // remove overlay interactivity
    gtk_window_set_accept_focus(GTK_WINDOW(overlay->overlay), FALSE);
    gtk_widget_set_sensitive(overlay->overlay, FALSE);
    g_signal_connect(G_OBJECT(overlay->overlay), "draw", G_CALLBACK(remove_input), NULL);

    // create container
    overlay->container = gtk_layout_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(overlay->overlay), overlay->container);

    return overlay;
}

// destroys and frees an overlay
void overlay_destroy(Overlay *overlay)
{
    // hide overlay
    overlay_hide(overlay);

    // remove tag references
    g_hash_table_unref(overlay->tags);

    // free overlay window
    gtk_widget_destroy(overlay->overlay);

    g_free(overlay);
}

// shows the overlay on top of the window
void overlay_show(Overlay *overlay, AtspiAccessible *window)
{
    // do nothing if invalid winow
    if (!window || overlay->window == window)
        return;

    // hide first
    overlay_hide(overlay);

    // set the new window
    overlay->window = g_object_ref(window);

    // show all tags
    GHashTableIter iter;
    gpointer tag_ptr, null_ptr;
    g_hash_table_iter_init(&iter, overlay->tags);
    while (g_hash_table_iter_next(&iter, &tag_ptr, &null_ptr))
        tag_show(tag_ptr, GTK_LAYOUT(overlay->container));

    // start the refresh loop
    overlay_refresh_loop(overlay);

    // show the window
    gtk_widget_show_all(overlay->overlay);
}

// hides the overlay and unsets the followed window
void overlay_hide(Overlay *overlay)
{
    // do nothing if no window
    if (!overlay->window)
        return;

    // remove the old window
    g_object_unref(overlay->window);
    overlay->window = NULL;

    // hide all tags
    GHashTableIter iter;
    gpointer tag_ptr, null_ptr;
    g_hash_table_iter_init(&iter, overlay->tags);
    while (g_hash_table_iter_next(&iter, &tag_ptr, &null_ptr))
        tag_hide(tag_ptr);

    // hide the overlay
    gtk_widget_hide(overlay->overlay);

    // remove idle refresh
    g_source_remove(overlay->refresh_source_id);
}

// adds a tag to the overlay
void overlay_add(Overlay *overlay, Tag *tag)
{
    // add tag reference
    gboolean added = g_hash_table_add(overlay->tags, tag);
    if (!added)
        return;

    // set shifted
    tag_shifted(tag, overlay->shifted);

    // show tag if overlay is shown
    if (overlay->window)
        tag_show(tag, GTK_LAYOUT(overlay->container));
}

// removes the tag from the overlay
void overlay_remove(Overlay *overlay, Tag *tag)
{
    // remove tag reference
    gboolean removed = g_hash_table_remove(overlay->tags, tag);
    if (!removed)
        return;

    // hide tag from overlay if shown
    if (overlay->window)
        tag_hide(tag);
}

// sets the overlay into the shifted state, which will apply it to the tags
void overlay_shifted(Overlay *overlay, gboolean shifted)
{
    // do nothing if same
    if (shifted == overlay->shifted)
        return;
    overlay->shifted = shifted;

    // update all tags
    GHashTableIter iter;
    gpointer tag_ptr, null_ptr;
    g_hash_table_iter_init(&iter, overlay->tags);
    while (g_hash_table_iter_next(&iter, &tag_ptr, &null_ptr))
        tag_shifted(tag_ptr, overlay->shifted);
}

// stops the overlay window from capturing mouse events
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

// repositions the overlay window and all its tags
static void overlay_refresh(Overlay *overlay)
{
    // do nothing if not shown
    if (!overlay->window)
        return;

    // reposition the overlay
    overlay_reposition(overlay);

    // reposition the tags
    GHashTableIter iter;
    gpointer tag_ptr, null_ptr;
    g_hash_table_iter_init(&iter, overlay->tags);
    while (g_hash_table_iter_next(&iter, &tag_ptr, &null_ptr))
        tag_reposition(tag_ptr);
}

// repositions the overlay over the followed window
static void overlay_reposition(Overlay *overlay)
{
    AtspiComponent *component = atspi_accessible_get_component_iface(overlay->window);
    AtspiRect *rect = atspi_component_get_extents(component, ATSPI_COORD_TYPE_SCREEN, NULL);

    gtk_window_move(GTK_WINDOW(overlay->overlay), rect->x, rect->y);
    gtk_window_resize(GTK_WINDOW(overlay->overlay), rect->width, rect->height);

    g_object_unref(component);
    g_free(rect);
}

// refreshes the overlay every interval
static gboolean overlay_refresh_loop(gpointer overlay_ptr)
{
    Overlay *overlay = overlay_ptr;

    // refresh the overlay
    overlay_refresh(overlay);

    // add a source to call in a bit
    overlay->refresh_source_id = g_timeout_add(OVERLAY_REFRESH_INTERVAL,
                                               overlay_refresh_loop,
                                               overlay);

    // remove this source
    return G_SOURCE_REMOVE;
}
