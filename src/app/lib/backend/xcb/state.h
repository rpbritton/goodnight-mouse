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

#ifndef A20B9782_EE13_4B14_AE33_AE273BE70B7F
#define A20B9782_EE13_4B14_AE33_AE273BE70B7F

#if USE_XCB

#include "xcb.h"
#include "../common/state.h"
#include "focus.h"

// backend for getting and parsing keyboard state
typedef struct BackendXCBState
{
    BackendXCB *backend;

    xcb_connection_t *connection;
    xcb_window_t root_window;
    xcb_input_device_id_t pointer_id;
} BackendXCBState;

BackendXCBState *backend_xcb_state_new(BackendXCB *backend);
void backend_xcb_state_destroy(BackendXCBState *state);
BackendStateEvent backend_xcb_state_current(BackendXCBState *state);

BackendStateEvent backend_xcb_state_parse(BackendXCBState *state, xcb_input_modifier_info_t mods, xcb_input_group_info_t group);

#endif /* USE_XCB */

#endif /* A20B9782_EE13_4B14_AE33_AE273BE70B7F */
