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

// create a new keyboard listener
BackendXCBKeyboard *backend_xcb_keyboard_new(BackendXCB *backend, BackendKeyboardCallback callback, gpointer data)
{
    BackendXCBKeyboard *keyboard = g_new(BackendXCBKeyboard, 1);

    // add backend
    keyboard->backend = backend;

    return keyboard;
}

// destroy the keyboard listener
void backend_xcb_keyboard_destroy(BackendXCBKeyboard *keyboard)
{
    // free
    g_free(keyboard);
}

// grab all keyboard input
void backend_xcb_keyboard_grab(BackendXCBKeyboard *keyboard)
{
    g_message("todo: backend_xcb_keyboard_grab");
}

// ungrab all keyboard input
void backend_xcb_keyboard_ungrab(BackendXCBKeyboard *keyboard)
{
    g_message("todo: backend_xcb_keyboard_ungrab");
}

// grab input of a specific key
void backend_xcb_keyboard_grab_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event)
{
    g_message("todo: backend_xcb_keyboard_grab_key");
}

// ungrab input of a specific key
void backend_xcb_keyboard_ungrab_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event)
{
    g_message("todo: backend_xcb_keyboard_ungrab_key");
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

#endif /* USE_XCB */
