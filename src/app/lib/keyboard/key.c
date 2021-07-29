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

#include "key.h"

#include "../timeout/timeout.h"

#include "modifiers.h"

// callback used to handle an atspi keyboard event
static gboolean atspi_callback(AtspiDeviceEvent *atspi_event, gpointer listener_ptr)
{
    KeyListener *listener = listener_ptr;
    KeyboardEvent event = keyboard_event_from_atspi(atspi_event);
    g_boxed_free(ATSPI_TYPE_DEVICE_EVENT, atspi_event);
    return listener->callback(event, listener->callback_data);
}

// listen for a particular key event
KeyListener *key_listener_new(KeyboardEvent event, KeyboardCallback callback, gpointer data)
{
    KeyListener *listener = g_new(KeyListener, 1);

    // add members
    listener->atspi_listener = atspi_device_listener_new(atspi_callback, listener, NULL);

    listener->atspi_key = g_array_sized_new(FALSE, TRUE, sizeof(AtspiKeyDefinition), 1);
    AtspiKeyDefinition atspi_key;
    memset(&atspi_key, 0, sizeof(AtspiKeyDefinition));
    atspi_key.keysym = event.key;
    g_array_append_val(listener->atspi_key, atspi_key);

    listener->atspi_type = event.type;
    listener->atspi_modifiers = keyboard_modifiers_map(event.modifiers);

    listener->callback = callback;
    listener->callback_data = data;

    // disable atspi timeout to avoid a deadlock with incoming key events
    timeout_disable();
    // register listener
    atspi_register_keystroke_listener(listener->atspi_listener,
                                      listener->atspi_key,
                                      listener->atspi_modifiers,
                                      listener->atspi_type,
                                      ATSPI_KEYLISTENER_SYNCHRONOUS | ATSPI_KEYLISTENER_CANCONSUME,
                                      NULL);
    // reenable the timeout
    timeout_enable();

    return listener;
}

// creates a new key event listener and starts listening
void key_listener_destroy(KeyListener *listener)
{
    // disable atspi timeout to avoid a deadlock with incoming key events
    timeout_disable();
    // deregister listener
    atspi_deregister_keystroke_listener(listener->atspi_listener,
                                        listener->atspi_key,
                                        listener->atspi_modifiers,
                                        listener->atspi_type,
                                        NULL);
    // reenable the timeout
    timeout_enable();

    // free members
    g_object_unref(listener->atspi_listener);
    g_array_unref(listener->atspi_key);

    // free
    g_free(listener);
}
