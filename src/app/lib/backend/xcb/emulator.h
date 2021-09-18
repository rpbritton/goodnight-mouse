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

#ifndef BC0B9014_1DE6_4245_A1EE_9FDB72126137
#define BC0B9014_1DE6_4245_A1EE_9FDB72126137

#if USE_XCB

#include "xcb.h"
#include "../common/state.h"
#include "../common/keyboard.h"
#include "state.h"

// backend for emulating events
typedef struct BackendXCBEmulator
{
    BackendXCB *backend;

    xcb_connection_t *connection;
    xcb_window_t root_window;

    xcb_input_device_id_t keyboard_id;
    xcb_input_device_id_t pointer_id;

    BackendXCBState *state;

    GHashTable *emulated_keys;
} BackendXCBEmulator;

BackendXCBEmulator *backend_xcb_emulator_new(BackendXCB *backend);
void backend_xcb_emulator_destroy(BackendXCBEmulator *emulator);
gboolean backend_xcb_emulator_reset(BackendXCBEmulator *emulator);
gboolean backend_xcb_emulator_state(BackendXCBEmulator *emulator, BackendStateEvent state);
gboolean backend_xcb_emulator_key(BackendXCBEmulator *emulator, BackendKeyboardEvent event);

#endif /* USE_XCB */

#endif /* BC0B9014_1DE6_4245_A1EE_9FDB72126137 */
