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

#ifndef C34EFBD7_588C_4399_8DEF_C92876EB3C3D
#define C34EFBD7_588C_4399_8DEF_C92876EB3C3D

#include <glib.h>

#include "state.h"

// whether the event should be passed through or not
typedef enum BackendKeyboardEventResponse
{
    BACKEND_KEYBOARD_EVENT_RELAY,
    BACKEND_KEYBOARD_EVENT_CONSUME,
} BackendKeyboardEventResponse;

// event representing a key action
typedef struct BackendKeyboardEvent
{
    guint keycode;
    gboolean pressed;
    BackendStateEvent state;
} BackendKeyboardEvent;

// callback used for keyboard events
typedef BackendKeyboardEventResponse (*BackendKeyboardCallback)(BackendKeyboardEvent event, gpointer data);

#endif /* C34EFBD7_588C_4399_8DEF_C92876EB3C3D */
