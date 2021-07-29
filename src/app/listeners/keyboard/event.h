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

#ifndef C4CF82D4_CE81_4D58_ABA2_434EBEA7A4FF
#define C4CF82D4_CE81_4D58_ABA2_434EBEA7A4FF

#include <glib.h>
#include <atspi/atspi.h>

// state change of a key in an event
typedef enum KeyboardEventType
{
    KEYBOARD_EVENT_PRESSED = (1 << ATSPI_KEY_PRESSED_EVENT),
    KEYBOARD_EVENT_RELEASED = (1 << ATSPI_KEY_RELEASED_EVENT),
} KeyboardEventType;

// how the event should be passed on
typedef enum KeyboardResponse
{
    KEYBOARD_EVENT_RELAY = FALSE,
    KEYBOARD_EVENT_CONSUME = TRUE,
} KeyboardResponse;

// a keyboard event
typedef struct KeyboardEvent
{
    guint key;
    KeyboardEventType type;
    guint modifiers;
} KeyboardEvent;

// callback type used when a subscribed keyboard event is fired
typedef KeyboardResponse (*KeyboardCallback)(KeyboardEvent event, gpointer data);

KeyboardEvent keyboard_event_from_atspi(AtspiDeviceEvent *atspi_event);

#endif /* C4CF82D4_CE81_4D58_ABA2_434EBEA7A4FF */
