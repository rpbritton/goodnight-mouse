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

#include "emulator.h"

#include <xcb/xtest.h>

#include "utils.h"

static gboolean emulate_modifiers(BackendXCBEmulator *emulator, guint8 modifiers);
static gboolean emulate_move(BackendXCBEmulator *emulator, gint x, gint y);
static gboolean emulate_detail(BackendXCBEmulator *emulator, guint8 type, guint8 detail, GHashTable *record);
static gboolean reset_details(BackendXCBEmulator *emulator, GHashTable *record);

// create a new emulator listener
BackendXCBEmulator *backend_xcb_emulator_new(BackendXCB *backend)
{
    BackendXCBEmulator *emulator = g_new(BackendXCBEmulator, 1);

    // add backend
    emulator->backend = backend;

    // add x connection
    emulator->connection = backend_xcb_get_connection(emulator->backend);
    emulator->root_window = backend_xcb_get_root_window(emulator->backend);

    // get device ids
    emulator->keyboard_id = backend_xcb_device_id_from_device_type(emulator->connection, XCB_INPUT_DEVICE_TYPE_MASTER_KEYBOARD);
    emulator->pointer_id = backend_xcb_device_id_from_device_type(emulator->connection, XCB_INPUT_DEVICE_TYPE_MASTER_POINTER);

    // new state
    emulator->state = backend_xcb_state_new(emulator->backend);

    // init emulated details
    emulator->emulated_keys = g_hash_table_new(NULL, NULL);
    emulator->emulated_buttons = g_hash_table_new(NULL, NULL);

    // return
    return emulator;
}

// destroy the emulator listener
void backend_xcb_emulator_destroy(BackendXCBEmulator *emulator)
{
    // free any currently emulated stuff
    backend_xcb_emulator_reset(emulator);
    g_hash_table_unref(emulator->emulated_keys);
    g_hash_table_unref(emulator->emulated_buttons);

    // free state
    backend_xcb_state_destroy(emulator->state);

    // free
    g_free(emulator);
}

// reset any emulated keys or state
gboolean backend_xcb_emulator_reset(BackendXCBEmulator *emulator)
{
    // reset any keys
    if (!reset_details(emulator, emulator->emulated_keys))
        return FALSE;

    // reset any buttons
    if (!reset_details(emulator, emulator->emulated_buttons))
        return FALSE;

    // return success
    return TRUE;
}

// set an emulated state
gboolean backend_xcb_emulator_state(BackendXCBEmulator *emulator, BackendStateEvent state)
{
    // todo: emulate group

    // emulate modifiers
    if (!emulate_modifiers(emulator, state.modifiers))
        return FALSE;

    // emulate pointer movement
    if (!emulate_move(emulator, state.pointer_x, state.pointer_y))
        return FALSE;

    // return success
    return TRUE;
}

// emulate a keyboard event
gboolean backend_xcb_emulator_key(BackendXCBEmulator *emulator, BackendKeyboardEvent event)
{
    // set the state
    if (!backend_xcb_emulator_state(emulator, event.state))
        return FALSE;

    // send the key event
    if (!emulate_detail(emulator, (event.pressed) ? XCB_KEY_PRESS : XCB_KEY_RELEASE, event.keycode, emulator->emulated_keys))
        return FALSE;

    // return success
    return TRUE;
}

// emulate a mouse event
gboolean backend_xcb_emulator_button(BackendXCBEmulator *emulator, BackendPointerEvent event)
{
    // set the state
    if (!backend_xcb_emulator_state(emulator, event.state))
        return FALSE;

    // send the key event
    if (!emulate_detail(emulator, (event.pressed) ? XCB_BUTTON_PRESS : XCB_BUTTON_RELEASE, event.button, emulator->emulated_buttons))
        return FALSE;

    // return success
    return TRUE;
}

// emulate the set of modifierse
static gboolean emulate_modifiers(BackendXCBEmulator *emulator, guint8 modifiers)
{
    // get current state
    BackendStateEvent current_state = backend_xcb_state_current(emulator->state);

    // send get modifiers mapping request
    xcb_input_get_device_modifier_mapping_cookie_t cookie;
    cookie = xcb_input_get_device_modifier_mapping(emulator->connection, emulator->keyboard_id);

    // get the reply
    xcb_generic_error_t *error = NULL;
    xcb_input_get_device_modifier_mapping_reply_t *reply;
    reply = xcb_input_get_device_modifier_mapping_reply(emulator->connection, cookie, &error);
    if (error)
    {
        g_warning("backend-xcb: Failed to get modifier mapping: error: %d", error->error_code);
        free(error);
        return FALSE;
    }
    if (!reply)
        return FALSE;

    // get the maps
    guint8 *modifier_keycodes = xcb_input_get_device_modifier_mapping_keymaps(reply);

    // check all the modifiers
    gboolean is_success = TRUE;
    for (gint mod_index = 0; mod_index <= 8; mod_index++)
    {
        // do nothing if already matches
        if ((modifiers & (1 << mod_index)) == (current_state.modifiers & (1 << mod_index)))
            continue;

        // get modifier action
        gboolean press = (modifiers & (1 << mod_index)) && !(current_state.modifiers & (1 << mod_index));

        // try sending keycodes to set the modifier
        for (gint keycode_index = 0; keycode_index < reply->keycodes_per_modifier; keycode_index++)
        {
            // get the keycode
            guint keycode = modifier_keycodes[mod_index * reply->keycodes_per_modifier + keycode_index];
            if (!keycode)
                continue;

            // send key
            emulate_detail(emulator, (press) ? XCB_KEY_PRESS : XCB_KEY_RELEASE, keycode, emulator->emulated_keys);

            // check if was set
            current_state = backend_xcb_state_current(emulator->state);
            if ((modifiers & (1 << mod_index)) == (current_state.modifiers & (1 << mod_index)))
                break;
            else
                emulate_detail(emulator, (!press) ? XCB_KEY_PRESS : XCB_KEY_RELEASE, keycode, emulator->emulated_keys);
        }

        // check if modifier was set
        if ((modifiers & (1 << mod_index)) != (current_state.modifiers & (1 << mod_index)))
            is_success = FALSE;
    }

    // return
    free(reply);
    return is_success;
}

// emulate the mouse movement
static gboolean emulate_move(BackendXCBEmulator *emulator, gint x, gint y)
{
    // send request
    xcb_void_cookie_t cookie;
    cookie = xcb_input_xi_warp_pointer_checked(emulator->connection,
                                               XCB_NONE,
                                               emulator->root_window,
                                               0, 0,
                                               0, 0,
                                               x << 16, y << 16,
                                               emulator->pointer_id);

    // get response
    xcb_generic_error_t *error = xcb_request_check(emulator->connection, cookie);
    if (error)
    {
        g_warning("backend-xcb: Failed to warp pointer: error: %d", error->error_code);
        free(error);
        return FALSE;
    }

    // return success
    return TRUE;
}

// emulate an input event
static gboolean emulate_detail(BackendXCBEmulator *emulator, guint8 type, guint8 detail, GHashTable *record)
{
    g_debug("backend-xcb: Emulating detail: type: %d, detail: %d", type, detail);

    // send key
    xcb_void_cookie_t cookie;
    // todo: deviceid can apparently be 0, will that still get picked up by grabs?
    cookie = xcb_test_fake_input_checked(emulator->connection,
                                         type,
                                         detail,
                                         XCB_CURRENT_TIME,
                                         emulator->root_window,
                                         0, 0,
                                         emulator->keyboard_id);

    // check response
    xcb_generic_error_t *error = xcb_request_check(emulator->connection, cookie);
    if (error)
    {
        g_warning("backend-xcb: Key emulation failed: type: %d, detail: %d, error %d",
                  type, detail, error->error_code);
        free(error);
        return FALSE;
    }

    // record in emulation table
    if (record)
    {
        if (g_hash_table_contains(record, GUINT_TO_POINTER(detail)))
        {
            if (GPOINTER_TO_UINT(g_hash_table_lookup(record, GUINT_TO_POINTER(detail))) != type)
                g_hash_table_remove(record, GUINT_TO_POINTER(detail));
        }
        else
        {
            g_hash_table_insert(record, GUINT_TO_POINTER(detail), GUINT_TO_POINTER(type));
        }
    }

    // return success
    return TRUE;
}

static gboolean reset_details(BackendXCBEmulator *emulator, GHashTable *record)
{
    if (!record)
        return TRUE;

    // get lists of keys to send
    gboolean is_success = TRUE;
    GHashTableIter iter;
    gpointer detail_ptr, type_ptr;
    g_hash_table_iter_init(&iter, record);
    while (g_hash_table_iter_next(&iter, &detail_ptr, &type_ptr))
    {
        switch (GPOINTER_TO_UINT(type_ptr))
        {
        case XCB_KEY_PRESS:
            is_success &= emulate_detail(emulator, XCB_KEY_RELEASE, GPOINTER_TO_UINT(detail_ptr), NULL);
            break;
        case XCB_KEY_RELEASE:
            is_success &= emulate_detail(emulator, XCB_KEY_PRESS, GPOINTER_TO_UINT(detail_ptr), NULL);
            break;
        case XCB_BUTTON_PRESS:
            is_success &= emulate_detail(emulator, XCB_BUTTON_RELEASE, GPOINTER_TO_UINT(detail_ptr), NULL);
            break;
        case XCB_BUTTON_RELEASE:
            is_success &= emulate_detail(emulator, XCB_BUTTON_PRESS, GPOINTER_TO_UINT(detail_ptr), NULL);
            break;
        default:
            is_success = FALSE;
        }
        g_hash_table_iter_remove(&iter);
    }

    // return
    return is_success;
}

#endif /* USE_XCB */
