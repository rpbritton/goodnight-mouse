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

#ifndef E0EEF1E6_8EE7_4E0E_94B1_7F1F3E5DAE25
#define E0EEF1E6_8EE7_4E0E_94B1_7F1F3E5DAE25

#include "backend.h"
#include "focus.h"

// whether the event should be passed through or not
typedef enum BackendXCBDeviceEventResponse
{
    BACKEND_XCB_DEVICE_EVENT_RELAY,
    BACKEND_XCB_DEVICE_EVENT_CONSUME,
} BackendXCBDeviceEventResponse;

// callback for xcb device events
typedef BackendXCBDeviceEventResponse (*BackendXCBDeviceCallback)(xcb_generic_event_t *generic_event, gpointer data);

// backend for listening and grabbing xcb device events
typedef struct BackendXCBDevice
{
    BackendXCB *backend;

    xcb_input_device_id_t device_id;
    guint32 event_mask;

    BackendXCBDeviceCallback callback;
    gpointer data;

    xcb_connection_t *connection;
    xcb_window_t root_window;

    gint device_grabs;
    GList *detail_grabs;

    BackendXCBFocus *focus;
    xcb_window_t grab_window;

    GHashTable *last_events;
} BackendXCBDevice;

BackendXCBDevice *backend_xcb_device_new(BackendXCB *backend, xcb_input_device_id_t device_id,
                                         guint32 event_mask, BackendXCBDeviceCallback callback, gpointer data);
void backend_xcb_device_destroy(BackendXCBDevice *device);
void backend_xcb_device_grab(BackendXCBDevice *device);
void backend_xcb_device_ungrab(BackendXCBDevice *device);
void backend_xcb_device_grab_detail(BackendXCBDevice *device, guint32 detail, guint32 modifiers);
void backend_xcb_device_ungrab_detail(BackendXCBDevice *device, guint32 detail, guint32 modifiers);
#endif /* E0EEF1E6_8EE7_4E0E_94B1_7F1F3E5DAE25 */
