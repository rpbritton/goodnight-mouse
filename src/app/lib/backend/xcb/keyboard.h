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
#include "../keyboard.h"
#include "device.h"
#include "state.h"

// backend for keyboard events
typedef struct BackendXCBKeyboard
{
    BackendXCB *backend;

    xcb_connection_t *connection;

    BackendKeyboardCallback callback;
    gpointer data;

    BackendXCBDevice *device;
    BackendXCBState *state;
} BackendXCBKeyboard;

BackendXCBKeyboard *backend_xcb_keyboard_new(BackendXCB *backend, BackendKeyboardCallback callback, gpointer data);
void backend_xcb_keyboard_destroy(BackendXCBKeyboard *keyboard);
void backend_xcb_keyboard_grab(BackendXCBKeyboard *keyboard);
void backend_xcb_keyboard_ungrab(BackendXCBKeyboard *keyboard);
void backend_xcb_keyboard_grab_key(BackendXCBKeyboard *keyboard, guint keycode, BackendStateEvent state);
void backend_xcb_keyboard_ungrab_key(BackendXCBKeyboard *keyboard, guint keycode, BackendStateEvent state);

#endif /* USE_XCB */

#endif /* FE6265C6_BCF0_41C2_B201_D16783856EB7 */
