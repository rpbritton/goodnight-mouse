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

#include "pointer.h"

static gboolean callback_pointer(AtspiDeviceEvent *atspi_event, gpointer pointer_ptr);

// create a new pointer listener
BackendLegacyPointer *backend_legacy_pointer_new(BackendLegacy *backend, BackendPointerCallback callback, gpointer data)
{
    BackendLegacyPointer *pointer = g_new(BackendLegacyPointer, 1);

    // add backend
    pointer->backend = backend;

    // add callback
    pointer->callback = callback;
    pointer->data = data;

    // register listener
    pointer->listener = atspi_device_listener_new(callback_pointer, pointer, NULL);
    atspi_register_device_event_listener(pointer->listener,
                                         (1 << ATSPI_BUTTON_PRESSED_EVENT) | (1 << ATSPI_BUTTON_RELEASED_EVENT),
                                         NULL,
                                         NULL);

    return pointer;
}

// destroy the pointer listener
void backend_legacy_pointer_destroy(BackendLegacyPointer *pointer)
{
    // deregister listener
    atspi_deregister_device_event_listener(pointer->listener, NULL, NULL);
    g_object_unref(pointer->listener);

    // free
    g_free(pointer);
}

// grab all pointer input
void backend_legacy_pointer_grab(BackendLegacyPointer *pointer)
{
    // do nothing, pointer is already grabbed by listener
}

// ungrab all pointer input
void backend_legacy_pointer_ungrab(BackendLegacyPointer *pointer)
{
    // do nothing, pointer is already grabbed by listener
}

// grab input of a specific button
void backend_legacy_pointer_grab_button(BackendLegacyPointer *pointer, guint buttoncode, BackendStateEvent state)
{
    // do nothing, pointer is already grabbed by listener
}

// ungrab input of a specific button
void backend_legacy_pointer_ungrab_button(BackendLegacyPointer *pointer, guint buttoncode, BackendStateEvent state)
{
    // do nothing, pointer is already grabbed by listener
}

static gboolean callback_pointer(AtspiDeviceEvent *atspi_event, gpointer pointer_ptr)
{
    BackendLegacyPointer *pointer = pointer_ptr;

    // get event
    BackendPointerEvent event;
    event.button = atspi_event->hw_code;
    event.pressed = (atspi_event->type == ATSPI_BUTTON_PRESSED_EVENT);
    event.state.modifiers = atspi_event->modifiers & 0xFF;
    event.state.group = 0; // todo, maybe contained in the gdk return bits 13 and 14?
    // todo: add mouse coordinates

    // free the atspi event
    g_boxed_free(ATSPI_TYPE_DEVICE_EVENT, atspi_event);

    // send the event
    BackendPointerEventResponse response = pointer->callback(event, pointer->data);

    // tell atspi whether to consume
    return (response == BACKEND_POINTER_EVENT_CONSUME) ? TRUE : FALSE;
}
