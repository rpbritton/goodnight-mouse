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

#include <gdk/gdk.h>

#include "state.h"

// create a new legacy state listener
BackendLegacyState *backend_legacy_state_new(BackendLegacy *backend)
{
    BackendLegacyState *state = g_new(BackendLegacyState, 1);

    // add backend
    state->backend = backend;

    // return
    return state;
}

// destroy the state listener
void backend_legacy_state_destroy(BackendLegacyState *state)
{
    // free
    g_free(state);
}

// get the current state
BackendStateEvent backend_legacy_state_current(BackendLegacyState *state)
{
    // get pointer position and modifiers
    GdkSeat *seat = gdk_display_get_default_seat(gdk_display_get_default());
    GdkDevice *gdk_mouse = gdk_seat_get_pointer(seat);
    GdkWindow *window = gdk_display_get_default_group(gdk_display_get_default());
    gint x, y;
    GdkModifierType modifiers;
    gdk_window_get_device_position(window, gdk_mouse, &x, &y, &modifiers);

    // create event
    // todo: group
    BackendStateEvent event = {
        .modifiers = modifiers & 0xFF,
        .pointer_x = x,
        .pointer_y = y,
    };

    // return
    return event;
}
