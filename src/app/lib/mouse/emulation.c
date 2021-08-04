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

#include "emulation.h"

#include <gdk/gdk.h>

gboolean mouse_press(Mouse *mouse, gint x, gint y, guint button)
{
    gboolean success = TRUE;

    // unregister mouse
    //gboolean registered = mouse_is_registered(mouse);
    //if (registered)
    //    mouse_deregister(mouse);

    // record the current mouse position
    GdkSeat *seat = gdk_display_get_default_seat(gdk_display_get_default());
    GdkDevice *gdk_mouse = gdk_seat_get_pointer(seat);
    GdkWindow *window = gdk_display_get_default_group(gdk_display_get_default());
    gint original_x, original_y;
    gdk_window_get_device_position(window, gdk_mouse, &original_x, &original_y, NULL);

    // move and click the mouse
    gchar *event_name = g_strdup_printf("b%dc", button);
    if (!atspi_generate_mouse_event(x, y, event_name, NULL))
        success = FALSE;
    g_free(event_name);

    // move the mouse back to original position
    if (!atspi_generate_mouse_event(original_x, original_y, "abs", NULL))
        success = FALSE;

    // reregister mouse
    //if (registered)
    //    mouse_register(mouse);

    // return success
    return success;
}
