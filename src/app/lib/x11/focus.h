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

#ifndef A2EF5F11_2529_4652_BC50_4EEE3926B412
#define A2EF5F11_2529_4652_BC50_4EEE3926B412

#if USE_X11

#include <atspi/atspi.h>
#include <X11/Xlib.h>

#include "backend.h"

#define backend_focus_new backend_x11_focus_new
#define backend_focus_destroy backend_x11_focus_destroy
#define backend_focus_get_window backend_x11_focus_get_window

// a callback for when the currently focused window changes, possibly to NULL
typedef void (*BackendX11FocusCallback)(AtspiAccessible *window, gpointer data);

typedef struct BackendX11Focus
{
    BackendX11 *backend;

    BackendX11FocusCallback callback;
    gpointer data;

    Display *display;
    Window root_window;
    AtspiEventListener *listener;
    AtspiAccessible *accessible;
} BackendX11Focus;

BackendX11Focus *backend_x11_focus_new(BackendX11 *backend, BackendX11FocusCallback callback, gpointer data);
void backend_x11_focus_destroy(BackendX11Focus *focus);
AtspiAccessible *backend_x11_focus_get_window(BackendX11Focus *focus);

#endif /* USE_X11 */

#endif /* A2EF5F11_2529_4652_BC50_4EEE3926B412 */
