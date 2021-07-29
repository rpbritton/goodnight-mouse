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

#include "mouse.h"

#include "../timeout/timeout.h"

// callback to handle an atspi mouse event
static gboolean atspi_callback(AtspiDeviceEvent *atspi_event, gpointer listener_ptr)
{
    MouseListener *listener = listener_ptr;
    MouseEvent event = mouse_event_from_atspi(atspi_event);
    g_boxed_free(ATSPI_TYPE_DEVICE_EVENT, atspi_event);
    return listener->callback(event, listener->callback_data);
}

// creates a new mouse event listener and starts listening
MouseListener *mouse_listener_new(MouseCallback callback, gpointer data)
{
    MouseListener *listener = g_new(MouseListener, 1);

    // add members
    listener->atspi_listener = atspi_device_listener_new(atspi_callback, listener, NULL);

    listener->callback = callback;
    listener->callback_data = data;

    // disable atspi timeout to avoid a deadlock with incoming key events
    timeout_disable();
    // register listener
    atspi_register_device_event_listener(listener->atspi_listener,
                                         MOUSE_EVENT_PRESSED | MOUSE_EVENT_RELEASED,
                                         NULL,
                                         NULL);
    // reenable the timeout
    timeout_enable();

    return listener;
}

// stops and destroys a mouse listener
void mouse_listener_destroy(MouseListener *listener)
{
    // disable atspi timeout to avoid a deadlock with incoming key events
    timeout_disable();
    // deregister listener
    atspi_deregister_device_event_listener(listener->atspi_listener, NULL, NULL);
    // reenable the timeout
    timeout_enable();

    // free members
    g_object_unref(listener->atspi_listener);

    // free
    g_free(listener);
}
