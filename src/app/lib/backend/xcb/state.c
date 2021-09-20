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

#if USE_XCB

#include "state.h"

#include "utils.h"

// create a new state
BackendXCBState *backend_xcb_state_new(BackendXCB *backend)
{
    BackendXCBState *state = g_new(BackendXCBState, 1);

    // add backend
    state->backend = backend;

    // add x connection
    state->connection = backend_xcb_get_connection(state->backend);
    state->root_window = backend_xcb_get_root_window(state->backend);

    // get the master pointer
    state->pointer_id = backend_xcb_device_id_from_device_type(state->connection, XCB_INPUT_DEVICE_TYPE_MASTER_POINTER);

    // return
    return state;
}

// destroy the state
void backend_xcb_state_destroy(BackendXCBState *state)
{
    // free
    g_free(state);
}

// get state
BackendStateEvent backend_xcb_state_current(BackendXCBState *state)
{
    // send request
    xcb_input_xi_query_pointer_cookie_t cookie;
    cookie = xcb_input_xi_query_pointer(state->connection, state->root_window, state->pointer_id);

    // get response
    xcb_generic_error_t *error = NULL;
    xcb_input_xi_query_pointer_reply_t *reply;
    reply = xcb_input_xi_query_pointer_reply(state->connection, cookie, &error);
    if (error != NULL)
    {
        g_warning("backend-xcb: Failed to query pointer: error: %d", error->error_code);
        free(error);
    }
    if (!reply)
    {
        BackendStateEvent event = {
            .modifiers = 0,
            .group = 0,
            .pointer_x = 0,
            .pointer_y = 0,
        };
        return event;
    }

    // get state
    BackendStateEvent event = backend_xcb_state_parse(state, reply->mods, reply->group, reply->root_x, reply->root_y);
    free(reply);
    return event;
}

// parse state from modifiers and group
BackendStateEvent backend_xcb_state_parse(BackendXCBState *state,
                                          xcb_input_modifier_info_t mods, xcb_input_group_info_t group,
                                          xcb_input_fp1616_t pointer_x, xcb_input_fp1616_t pointer_y)
{
    // xcb has a bug where the effective modifiers are not computed sometimes (e.g. in the query pointer response)
    BackendStateEvent event = {
        .modifiers = mods.base | mods.latched | mods.locked | mods.effective,
        .group = group.base | group.latched | group.locked | group.effective,
        .pointer_x = pointer_x >> 16,
        .pointer_y = pointer_y >> 16,
    };
    return event;
}

#endif /* USE_XCB */
