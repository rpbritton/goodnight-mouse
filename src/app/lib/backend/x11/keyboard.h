/**
 * Copyright (C) 2021 ryan
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

#ifndef F57F1019_9CFE_4F5D_A723_17B8C671BC05
#define F57F1019_9CFE_4F5D_A723_17B8C671BC05

#if USE_X11

#include "x11.h"
#include "xinput.h"
#include "../common/keyboard.h"

// backend for keyboard events that uses pure atspi
typedef struct BackendX11Keyboard
{
    BackendX11 *backend;

    BackendKeyboardCallback callback;
    gpointer data;

    Display *display;
    Window root_window;

    int keyboard_id;
    int pointer_id;

    BackendX11XInput *xinput;
} BackendX11Keyboard;

BackendX11Keyboard *backend_x11_keyboard_new(BackendX11 *backend, BackendKeyboardCallback callback, gpointer data);
void backend_x11_keyboard_destroy(BackendX11Keyboard *keyboard);
void backend_x11_keyboard_grab(BackendX11Keyboard *keyboard);
void backend_x11_keyboard_ungrab(BackendX11Keyboard *keyboard);
void backend_x11_keyboard_grab_key(BackendX11Keyboard *keyboard, KeyboardEvent event);
void backend_x11_keyboard_ungrab_key(BackendX11Keyboard *keyboard, KeyboardEvent event);
Modifiers backend_x11_keyboard_get_modifiers(BackendX11Keyboard *keyboard);

#endif /* USE_X11 */

#endif /* F57F1019_9CFE_4F5D_A723_17B8C671BC05 */
