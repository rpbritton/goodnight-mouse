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

#ifndef CA346DEE_BC05_4A17_9208_F0135AA77F7A
#define CA346DEE_BC05_4A17_9208_F0135AA77F7A

#include <glib.h>
#include <atspi/atspi.h>

// state change of a button in an event
typedef enum MouseEventType
{
    MOUSE_EVENT_PRESSED = (1 << ATSPI_BUTTON_PRESSED_EVENT),
    MOUSE_EVENT_RELEASED = (1 << ATSPI_BUTTON_RELEASED_EVENT),
} MouseEventType;

// how the event should be passed on
typedef enum MouseResponse
{
    MOUSE_EVENT_RELAY = FALSE,
    MOUSE_EVENT_CONSUME = TRUE,
} MouseResponse;

// a mouse event
typedef struct MouseEvent
{
    guint button;
    MouseEventType type;
} MouseEvent;

// callback type used when a subscribed mouse event is fired
typedef MouseResponse (*MouseCallback)(MouseEvent event, gpointer data);

MouseEvent mouse_event_from_atspi(AtspiDeviceEvent *atspi_event);

#endif /* CA346DEE_BC05_4A17_9208_F0135AA77F7A */
