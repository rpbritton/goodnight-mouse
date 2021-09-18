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

#if USE_XCB

#include "pointer.h"

#include "utils.h"

static BackendXCBDeviceEventResponse callback_device(xcb_generic_event_t *generic_event, gpointer pointer_ptr);

// create a new pointer listener
BackendXCBPointer *backend_xcb_pointer_new(BackendXCB *backend, BackendPointerCallback callback, gpointer data)
{
    BackendXCBPointer *pointer = g_new(BackendXCBPointer, 1);

    // add backend
    pointer->backend = backend;

    // add connection
    pointer->connection = backend_xcb_get_connection(pointer->backend);

    // add callback
    pointer->callback = callback;
    pointer->data = data;

    // add device
    xcb_input_device_id_t pointer_id = backend_xcb_device_id_from_device_type(pointer->connection,
                                                                              XCB_INPUT_DEVICE_TYPE_MASTER_POINTER);
    pointer->device = backend_xcb_device_new(pointer->backend,
                                             pointer_id,
                                             (XCB_INPUT_XI_EVENT_MASK_BUTTON_PRESS | XCB_INPUT_XI_EVENT_MASK_BUTTON_RELEASE),
                                             callback_device,
                                             pointer);

    // add state
    pointer->state = backend_xcb_state_new(pointer->backend);

    // return
    return pointer;
}

// destroy the pointer listener
void backend_xcb_pointer_destroy(BackendXCBPointer *pointer)
{
    // free device
    backend_xcb_device_destroy(pointer->device);

    // free state
    backend_xcb_state_destroy(pointer->state);

    // free
    g_free(pointer);
}

// grab all pointer input
void backend_xcb_pointer_grab(BackendXCBPointer *pointer)
{
    backend_xcb_device_grab(pointer->device);
}

// ungrab all pointer input
void backend_xcb_pointer_ungrab(BackendXCBPointer *pointer)
{
    backend_xcb_device_ungrab(pointer->device);
}

// grab input of a specific button
void backend_xcb_pointer_grab_button(BackendXCBPointer *pointer, BackendPointerEvent event)
{
    // todo: include group
    backend_xcb_device_grab_detail(pointer->device, event.button, event.state.modifiers);
}

// ungrab input of a specific button
void backend_xcb_pointer_ungrab_button(BackendXCBPointer *pointer, BackendPointerEvent event)
{
    // todo: include group
    backend_xcb_device_ungrab_detail(pointer->device, event.button, event.state.modifiers);
}

// callback for handling button events
static BackendXCBDeviceEventResponse callback_device(xcb_generic_event_t *generic_event, gpointer pointer_ptr)
{
    BackendXCBPointer *pointer = pointer_ptr;

    // get button event
    xcb_input_button_press_event_t *button_event = (xcb_input_button_press_event_t *)generic_event;

    g_message("got button event: root_x: %d, root_y: %d, event_x: %d, event_y: %d",
              button_event->root_x, button_event->root_y, button_event->event_x, button_event->event_y);

    // get event data
    BackendPointerEvent event;
    event.button = button_event->detail;
    event.pressed = (button_event->event_type == XCB_INPUT_BUTTON_PRESS);
    event.state = backend_xcb_state_parse(pointer->state, button_event->mods, button_event->group);

    // send the event
    BackendPointerEventResponse response = pointer->callback(event, pointer->data);

    // return the response
    return (response == BACKEND_POINTER_EVENT_CONSUME) ? BACKEND_XCB_DEVICE_EVENT_CONSUME
                                                       : BACKEND_XCB_DEVICE_EVENT_RELAY;
}

#endif /* USE_XCB */
