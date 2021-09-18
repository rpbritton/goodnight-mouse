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

#include "keyboard.h"

#include "utils.h"

static BackendXCBDeviceEventResponse callback_device(xcb_generic_event_t *generic_event, gpointer keyboard_ptr);

// create a new keyboard listener
BackendXCBKeyboard *backend_xcb_keyboard_new(BackendXCB *backend, BackendKeyboardCallback callback, gpointer data)
{
    BackendXCBKeyboard *keyboard = g_new(BackendXCBKeyboard, 1);

    // add backend
    keyboard->backend = backend;

    // add x connection
    keyboard->connection = backend_xcb_get_connection(keyboard->backend);

    // add callback
    keyboard->callback = callback;
    keyboard->data = data;

    // add device
    xcb_input_device_id_t keyboard_id = backend_xcb_device_id_from_device_type(keyboard->connection,
                                                                               XCB_INPUT_DEVICE_TYPE_MASTER_KEYBOARD);
    keyboard->device = backend_xcb_device_new(keyboard->backend,
                                              keyboard_id,
                                              (XCB_INPUT_XI_EVENT_MASK_KEY_PRESS | XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE),
                                              callback_device,
                                              keyboard);

    // add state
    keyboard->state = backend_xcb_state_new(keyboard->backend);

    // return
    return keyboard;
}

// destroy the keyboard listener
void backend_xcb_keyboard_destroy(BackendXCBKeyboard *keyboard)
{
    // free device
    backend_xcb_device_destroy(keyboard->device);

    // free state
    backend_xcb_state_destroy(keyboard->state);

    // free
    g_free(keyboard);
}

// grab all keyboard input
void backend_xcb_keyboard_grab(BackendXCBKeyboard *keyboard)
{
    backend_xcb_device_grab(keyboard->device);
}

// ungrab all keyboard input
void backend_xcb_keyboard_ungrab(BackendXCBKeyboard *keyboard)
{
    backend_xcb_device_ungrab(keyboard->device);
}

// grab input of a specific key
void backend_xcb_keyboard_grab_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event)
{
    // todo: include group
    backend_xcb_device_grab_detail(keyboard->device, event.keycode, event.state.modifiers);
}

// ungrab input of a specific key
void backend_xcb_keyboard_ungrab_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event)
{
    // todo: include group
    backend_xcb_device_ungrab_detail(keyboard->device, event.keycode, event.state.modifiers);
}

// callback for handling key events
static BackendXCBDeviceEventResponse callback_device(xcb_generic_event_t *generic_event, gpointer keyboard_ptr)
{
    BackendXCBKeyboard *keyboard = keyboard_ptr;

    // get key event
    xcb_input_key_press_event_t *key_event = (xcb_input_key_press_event_t *)generic_event;

    // get event data
    BackendKeyboardEvent event;
    event.keycode = key_event->detail;
    event.pressed = (key_event->event_type == XCB_INPUT_KEY_PRESS);
    event.state = backend_xcb_state_parse(keyboard->state, key_event->mods, key_event->group);

    // send the event
    BackendKeyboardEventResponse response = keyboard->callback(event, keyboard->data);

    // return the response
    return (response == BACKEND_KEYBOARD_EVENT_CONSUME) ? BACKEND_XCB_DEVICE_EVENT_CONSUME
                                                        : BACKEND_XCB_DEVICE_EVENT_RELAY;
}

#endif /* USE_XCB */
