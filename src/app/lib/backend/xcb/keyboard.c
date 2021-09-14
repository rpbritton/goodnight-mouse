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

static void callback_key_event(xcb_generic_event_t *generic_event, gpointer keyboard_ptr);

// create a new keyboard listener
BackendXCBKeyboard *backend_xcb_keyboard_new(BackendXCB *backend, BackendKeyboardCallback callback, gpointer data)
{
    BackendXCBKeyboard *keyboard = g_new(BackendXCBKeyboard, 1);

    // add backend
    keyboard->backend = backend;

    // add callback
    keyboard->callback = callback;
    keyboard->data = data;

    // add x connection
    keyboard->connection = backend_xcb_get_connection(keyboard->backend);
    keyboard->root = backend_xcb_get_root(keyboard->backend);

    // initialize grabs
    keyboard->grabs = 0;
    keyboard->key_grabs = NULL;

    // subscribe to key events
    backend_xcb_subscribe(keyboard->backend, BACKEND_XCB_EXTENSION_XINPUT, XCB_INPUT_KEY_PRESS, callback_key_event, keyboard);
    backend_xcb_subscribe(keyboard->backend, BACKEND_XCB_EXTENSION_XINPUT, XCB_INPUT_KEY_RELEASE, callback_key_event, keyboard);

    return keyboard;
}

// destroy the keyboard listener
void backend_xcb_keyboard_destroy(BackendXCBKeyboard *keyboard)
{
    // unsubscribe from key events
    backend_xcb_unsubscribe(keyboard->backend, BACKEND_XCB_EXTENSION_XINPUT, XCB_INPUT_KEY_PRESS, callback_key_event, keyboard);
    backend_xcb_unsubscribe(keyboard->backend, BACKEND_XCB_EXTENSION_XINPUT, XCB_INPUT_KEY_RELEASE, callback_key_event, keyboard);

    // free
    g_free(keyboard);
}

// grab all keyboard input
void backend_xcb_keyboard_grab(BackendXCBKeyboard *keyboard)
{
    // add a grab
    keyboard->grabs++;

    // add grab if not set
    if (keyboard->grabs == 1)
        g_message("todo: set_grab");
}

// ungrab all keyboard input
void backend_xcb_keyboard_ungrab(BackendXCBKeyboard *keyboard)
{
    // check if the grab exists
    if (keyboard->grabs == 0)
        return;

    // remove a grab
    keyboard->grabs--;

    // remove the grab if none exist now
    if (keyboard->grabs == 0)
        g_message("todo: unset_grab");
}

// grab input of a specific key
void backend_xcb_keyboard_grab_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event)
{
    // allocate grab
    BackendKeyboardEvent *grab = g_new(BackendKeyboardEvent, 1);
    *grab = event;

    // add grab
    keyboard->key_grabs = g_list_append(keyboard->key_grabs, grab);

    // reset grab
    g_message("todo: reset_key_grab");
}

// ungrab input of a specific key
void backend_xcb_keyboard_ungrab_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event)
{
    // remove the first instance of the grab
    for (GList *link = keyboard->key_grabs; link; link = link->next)
    {
        BackendKeyboardEvent *grab = link->data;

        // check if grab matches
        if (!((grab->keycode == event.keycode) &&
              (grab->state.modifiers == event.state.modifiers) &&
              (grab->state.group == event.state.group)))
            continue;

        // remove grab
        keyboard->key_grabs = g_list_delete_link(keyboard->key_grabs, link);

        // reset grab
        g_message("todo: reset_key_grab");

        // free grab
        g_free(grab);
        return;
    }
}

// get keyboard state
BackendKeyboardState backend_xcb_keyboard_get_state(BackendXCBKeyboard *keyboard)
{
    g_message("todo: backend_xcb_keyboard_get_state");

    BackendKeyboardState state = {
        .group = 0,
        .modifiers = 0,
    };
    return state;
}

// reset any emulated keys or state
void backend_xcb_keyboard_emulate_reset(BackendXCBKeyboard *keyboard)
{
    g_message("todo: backend_xcb_keyboard_emulate_reset");
}

// set an emulated state
void backend_xcb_keyboard_emulate_state(BackendXCBKeyboard *keyboard, BackendKeyboardState state)
{
    g_message("todo: backend_xcb_keyboard_emulate_state");
}

// set an emulated key
void backend_xcb_keyboard_emulate_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event)
{
    g_message("todo: backend_xcb_keyboard_emulate_key");
}

static void callback_key_event(xcb_generic_event_t *generic_event, gpointer keyboard_ptr)
{
    // get key event
    xcb_input_key_press_event_t *event = (xcb_input_key_press_event_t *)generic_event;
    g_message("got key: press: %d, detail: %d, mods: %d", event->event_type == XCB_INPUT_KEY_PRESS, event->detail, event->mods.effective);
    // todo: are you freeing events correctly?
}

#endif /* USE_XCB */
