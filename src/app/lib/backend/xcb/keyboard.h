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

#ifndef FE6265C6_BCF0_41C2_B201_D16783856EB7
#define FE6265C6_BCF0_41C2_B201_D16783856EB7

#if USE_XCB

#include "xcb.h"
#include "../common/keyboard.h"

// backend for keyboard events that uses pure atspi
typedef struct BackendXCBKeyboard
{
    BackendXCB *backend;

    BackendKeyboardCallback callback;
    gpointer data;

    xcb_connection_t *connection;
    xcb_window_t root;

    gint grabs;
    GList *key_grabs;
} BackendXCBKeyboard;

BackendXCBKeyboard *backend_xcb_keyboard_new(BackendXCB *backend, BackendKeyboardCallback callback, gpointer data);
void backend_xcb_keyboard_destroy(BackendXCBKeyboard *keyboard);
void backend_xcb_keyboard_grab(BackendXCBKeyboard *keyboard);
void backend_xcb_keyboard_ungrab(BackendXCBKeyboard *keyboard);
void backend_xcb_keyboard_grab_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event);
void backend_xcb_keyboard_ungrab_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event);
BackendKeyboardState backend_xcb_keyboard_get_state(BackendXCBKeyboard *keyboard);
void backend_xcb_keyboard_emulate_reset(BackendXCBKeyboard *keyboard);
void backend_xcb_keyboard_emulate_state(BackendXCBKeyboard *keyboard, BackendKeyboardState state);
void backend_xcb_keyboard_emulate_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event);

#endif /* USE_XCB */

#endif /* FE6265C6_BCF0_41C2_B201_D16783856EB7 */
