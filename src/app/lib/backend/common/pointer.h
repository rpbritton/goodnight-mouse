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

#ifndef C18A4891_9E78_4E59_8C06_6BD9C6108DA3
#define C18A4891_9E78_4E59_8C06_6BD9C6108DA3

#include <glib.h>

#include "state.h"

// whether the event should be passed through or not
typedef enum BackendPointerEventResponse
{
    BACKEND_POINTER_EVENT_RELAY,
    BACKEND_POINTER_EVENT_CONSUME,
} BackendPointerEventResponse;

// event representing a key action
typedef struct BackendPointerEvent
{
    guint button;
    gboolean pressed;
    BackendStateEvent state;
} BackendPointerEvent;

// callback used for pointer events
typedef BackendPointerEventResponse (*BackendPointerCallback)(BackendPointerEvent event, gpointer data);

#endif /* C18A4891_9E78_4E59_8C06_6BD9C6108DA3 */
