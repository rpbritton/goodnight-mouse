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

#ifndef BDF15D43_14FE_4926_8658_DE1AECF65525
#define BDF15D43_14FE_4926_8658_DE1AECF65525

#if USE_XCB

#include <glib.h>
#include <xcb/xcb.h>

#include "../legacy/legacy.h"

// subscriber callback to xcb events
typedef void (*BackendXCBCallback)(xcb_generic_event_t *event, gpointer data);

// structure for the xcb backend
typedef struct BackendXCB
{
    xcb_connection_t *connection;
    xcb_window_t root;
    GSource *source;

    BackendLegacy *legacy;

    GList *subscribers;
} BackendXCB;

BackendXCB *backend_xcb_new();
void backend_xcb_destroy(BackendXCB *backend);

void backend_xcb_subscribe(BackendXCB *backend, guint8 type, BackendXCBCallback callback, gpointer data);
void backend_xcb_unsubscribe(BackendXCB *backend, guint8 type, BackendXCBCallback callback, gpointer data);
xcb_connection_t *backend_xcb_get_connection(BackendXCB *backend);
xcb_window_t backend_xcb_get_root(BackendXCB *backend);
BackendLegacy *backend_xcb_get_legacy(BackendXCB *backend);

#endif /* USE_XCB */

#endif /* BDF15D43_14FE_4926_8658_DE1AECF65525 */