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

static gboolean emulate_key(BackendXCBEmulator *emulator, guint keycode, gboolean pressed);

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

    // init emulated keys
    emulator->emulated_keys = g_hash_table_new(NULL, NULL);

    // return
    return emulator;
}

// destroy the emulator listener
void backend_xcb_emulator_destroy(BackendXCBEmulator *emulator)
{
    // free any currently emulated stuff
    backend_xcb_emulator_reset(emulator);
    g_hash_table_unref(emulator->emulated_keys);

    // free state
    backend_xcb_state_destroy(emulator->state);

    // free
    g_free(emulator);
}

// reset any emulated keys or state
gboolean backend_xcb_emulator_reset(BackendXCBEmulator *emulator)
{
    // get lists of keys to send
    GList *keys_to_press = NULL;
    GList *keys_to_release = NULL;
    GHashTableIter iter;
    gpointer keycode, pressed;
    g_hash_table_iter_init(&iter, emulator->emulated_keys);
    while (g_hash_table_iter_next(&iter, &keycode, &pressed))
        if (GPOINTER_TO_UINT(pressed))
            keys_to_release = g_list_prepend(keys_to_release, keycode);
        else
            keys_to_press = g_list_prepend(keys_to_press, keycode);

    // send keys
    gboolean is_success = TRUE;
    for (GList *link = keys_to_press; link; link = link->next)
        is_success &= emulate_key(emulator, GPOINTER_TO_UINT(link->data), TRUE);
    g_list_free(keys_to_press);
    for (GList *link = keys_to_release; link; link = link->next)
        is_success &= emulate_key(emulator, GPOINTER_TO_UINT(link->data), FALSE);
    g_list_free(keys_to_release);

    // return
    return is_success;
}

// set an emulated state
gboolean backend_xcb_emulator_state(BackendXCBEmulator *emulator, BackendStateEvent state)
{
    // todo: use group

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
    guint8 *modifiers = xcb_input_get_device_modifier_mapping_keymaps(reply);

    // check all the modifiers
    gboolean is_success = TRUE;
    for (gint mod_index = 0; mod_index <= 8; mod_index++)
    {
        // do nothing if already matches
        if ((state.modifiers & (1 << mod_index)) == (current_state.modifiers & (1 << mod_index)))
            continue;

        // get modifier action
        gboolean press = (state.modifiers & (1 << mod_index)) && !(current_state.modifiers & (1 << mod_index));

        // try sending keycodes to set the modifier
        for (gint keycode_index = 0; keycode_index < reply->keycodes_per_modifier; keycode_index++)
        {
            // get the keycode
            guint keycode = modifiers[mod_index * reply->keycodes_per_modifier + keycode_index];
            if (!keycode)
                continue;

            // send key
            emulate_key(emulator, keycode, press);

            // check if was set
            current_state = backend_xcb_state_current(emulator->state);
            if ((state.modifiers & (1 << mod_index)) == (current_state.modifiers & (1 << mod_index)))
                break;
            else
                emulate_key(emulator, keycode, !press);
        }

        // check if modifier was set
        if ((state.modifiers & (1 << mod_index)) != (current_state.modifiers & (1 << mod_index)))
            is_success = FALSE;
    }

    // return
    free(reply);
    return is_success;
}

// set an emulated key
gboolean backend_xcb_emulator_key(BackendXCBEmulator *emulator, BackendKeyboardEvent event)
{
    // set the state
    if (!backend_xcb_emulator_state(emulator, event.state))
        return FALSE;

    // send the key event
    if (!emulate_key(emulator, event.keycode, event.pressed))
        return FALSE;

    // return success
    return TRUE;
}

static gboolean emulate_key(BackendXCBEmulator *emulator, guint keycode, gboolean press)
{
    g_debug("backend-xcb: Emulating key: keycode: %d, press: %d", keycode, press);

    // send key
    xcb_void_cookie_t cookie;
    // todo: deviceid can apparently be 0, will that still get picked up by grabs?
    cookie = xcb_test_fake_input_checked(emulator->connection,
                                         (press) ? XCB_KEY_PRESS : XCB_KEY_RELEASE,
                                         keycode,
                                         XCB_CURRENT_TIME,
                                         emulator->root_window,
                                         0, 0,
                                         emulator->keyboard_id);

    // check response
    xcb_generic_error_t *error = xcb_request_check(emulator->connection, cookie);
    if (error)
    {
        g_warning("backend-xcb: Key emulation failed: keycode: %d, press: %d, error %d",
                  keycode, press, error->error_code);
        free(error);
        return FALSE;
    }

    // record in emulation table
    if (g_hash_table_contains(emulator->emulated_keys, GUINT_TO_POINTER(keycode)))
    {
        if (GPOINTER_TO_UINT(g_hash_table_lookup(emulator->emulated_keys, GUINT_TO_POINTER(keycode))) != press)
            g_hash_table_remove(emulator->emulated_keys, GUINT_TO_POINTER(keycode));
    }
    else
    {
        g_hash_table_insert(emulator->emulated_keys, GUINT_TO_POINTER(keycode), GUINT_TO_POINTER(press));
    }

    // return success
    return TRUE;
}

#endif /* USE_XCB */
