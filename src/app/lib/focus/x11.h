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

#ifndef E08E96C5_9A2C_464D_AA33_833B2045A3CF
#define E08E96C5_9A2C_464D_AA33_833B2045A3CF

#if USE_X11

#include <atspi/atspi.h>
#include <X11/Xlib.h>

#include "event.h"

typedef struct FocusX11
{
    FocusCallback callback;
    gpointer data;

    Display *display;
    Window root_window;
    AtspiAccessible *accessible;
} FocusX11;

FocusX11 *focus_x11_new(FocusCallback callback, gpointer data);
void focus_x11_destroy(FocusX11 *focus_x11);
AtspiAccessible *focus_x11_get_window(FocusX11 *focus_x11);

#endif /* USE_X11 */

#endif /* E08E96C5_9A2C_464D_AA33_833B2045A3CF */
