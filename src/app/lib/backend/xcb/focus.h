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

#ifndef B53CADD9_4B91_408B_B0DE_DF18356B7745
#define B53CADD9_4B91_408B_B0DE_DF18356B7745

#if USE_XCB

#include <atspi/atspi.h>

#include "xcb.h"
#include "../common/focus.h"
#include "../legacy/focus.h"

// backend for focus that uses xcb
typedef struct BackendXCBFocus
{
    BackendXCB *backend;

    BackendFocusCallback callback;
    gpointer data;

    xcb_connection_t *connection;
    xcb_window_t root;

    xcb_atom_t atom_active_window;
    xcb_atom_t atom_window_pid;

    BackendLegacyFocus *legacy;
} BackendXCBFocus;

BackendXCBFocus *backend_xcb_focus_new(BackendXCB *backend, BackendFocusCallback callback, gpointer data);
void backend_xcb_focus_destroy(BackendXCBFocus *focus);
AtspiAccessible *backend_xcb_focus_get_window(BackendXCBFocus *focus);

xcb_window_t backend_xcb_focus_get_xcb_window(BackendXCBFocus *focus);

#endif /* USE_XCB */

#endif /* B53CADD9_4B91_408B_B0DE_DF18356B7745 */
