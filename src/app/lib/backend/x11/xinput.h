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

#ifndef D7077F17_9E49_4050_AB22_C32909066663
#define D7077F17_9E49_4050_AB22_C32909066663

#if USE_X11

#include <X11/extensions/XInput2.h>

#include "x11.h"

// callback for when window focus changes
typedef void (*BackendX11XInputCallback)(XIDeviceEvent *event, gpointer data);

// xinput wrapper for the x11 backend
typedef struct BackendX11XInput
{
    BackendX11 *backend;

    BackendX11XInputCallback callback;
    gpointer data;

    Display *display;
    int opcode;
} BackendX11XInput;

BackendX11XInput *backend_x11_xinput_new(BackendX11 *backend, BackendX11XInputCallback callback, gpointer data);
void backend_x11_xinput_destroy(BackendX11XInput *xinput);

#endif /* USE_X11 */

#endif /* D7077F17_9E49_4050_AB22_C32909066663 */
