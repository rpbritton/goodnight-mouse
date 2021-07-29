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

#include "keyboard.h"

#include "../timeout/timeout.h"

// callback to handle an atspi keyboard event
static gboolean atspi_callback(AtspiDeviceEvent *atspi_event, gpointer listener_ptr)
{
    KeyboardListener *listener = listener_ptr;
    KeyboardEvent event = keyboard_event_from_atspi(atspi_event);
    g_boxed_free(ATSPI_TYPE_DEVICE_EVENT, atspi_event);
    return listener->callback(event, listener->callback_data);
}

// creates a new keyboard event listener and starts listening
KeyboardListener *keyboard_listener_new(KeyboardCallback callback, gpointer data)
{
    KeyboardListener *listener = g_new(KeyboardListener, 1);

    // add callback
    listener->callback = callback;
    listener->callback_data = data;

    // create listener
    listener->atspi_listener = atspi_device_listener_new(atspi_callback, listener, NULL);
    // disable atspi timeout to avoid a deadlock with incoming key events
    timeout_disable();
    // register listener
    for (gint modifiers = 0; modifiers < 0xFF; modifiers++)
        atspi_register_keystroke_listener(listener->atspi_listener,
                                          NULL,
                                          modifiers,
                                          KEYBOARD_EVENT_PRESSED | KEYBOARD_EVENT_RELEASED,
                                          ATSPI_KEYLISTENER_SYNCHRONOUS | ATSPI_KEYLISTENER_CANCONSUME,
                                          NULL);
    // reenable the timeout
    timeout_enable();

    return listener;
}

// stops and destroys a keyboard listener
void keyboard_listener_destroy(KeyboardListener *listener)
{
    // disable atspi timeout to avoid a deadlock with incoming key events
    timeout_disable();
    // deregister listener
    for (gint modifiers = 0; modifiers < 0xFF; modifiers++)
        atspi_deregister_keystroke_listener(listener->atspi_listener,
                                            NULL,
                                            modifiers,
                                            KEYBOARD_EVENT_PRESSED | KEYBOARD_EVENT_RELEASED,
                                            NULL);
    // reenable the timeout
    timeout_enable();

    // free members
    g_object_unref(listener->atspi_listener);

    // free
    g_free(listener);
}
