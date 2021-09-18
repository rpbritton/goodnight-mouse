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

#ifndef CCB12CE1_7AD2_4E40_9D80_F7D93BD744F9
#define CCB12CE1_7AD2_4E40_9D80_F7D93BD744F9

#if USE_XCB

#include "xcb.h"
#include "../common/pointer.h"
#include "device.h"
#include "state.h"

// backend for pointer events
typedef struct BackendXCBPointer
{
    BackendXCB *backend;

    xcb_connection_t *connection;

    BackendPointerCallback callback;
    gpointer data;

    BackendXCBDevice *device;
    BackendXCBState *state;
} BackendXCBPointer;

BackendXCBPointer *backend_xcb_pointer_new(BackendXCB *backend, BackendPointerCallback callback, gpointer data);
void backend_xcb_pointer_destroy(BackendXCBPointer *pointer);
void backend_xcb_pointer_grab(BackendXCBPointer *pointer);
void backend_xcb_pointer_ungrab(BackendXCBPointer *pointer);
void backend_xcb_pointer_grab_button(BackendXCBPointer *pointer, BackendPointerEvent event);
void backend_xcb_pointer_ungrab_button(BackendXCBPointer *pointer, BackendPointerEvent event);

#endif /* USE_XCB */

#endif /* CCB12CE1_7AD2_4E40_9D80_F7D93BD744F9 */
