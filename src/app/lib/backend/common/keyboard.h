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

// type of key event
typedef enum KeyboardEventType
{
    KEYBOARD_EVENT_PRESSED = (1 << 0),
    KEYBOARD_EVENT_RELEASED = (1 << 1),
} KeyboardEventType;

// type for representing modifiers
typedef guchar Modifiers;

// event representing a key action
typedef struct KeyboardEvent
{
    guint keysym;
    KeyboardEventType type;
    Modifiers modifiers;
} KeyboardEvent;

// callback used for keyboard events
typedef void (*BackendKeyboardCallback)(KeyboardEvent event, gpointer data);

void backend_keyboard_sanitize_event(KeyboardEvent *event);

#endif /* C34EFBD7_588C_4399_8DEF_C92876EB3C3D */
