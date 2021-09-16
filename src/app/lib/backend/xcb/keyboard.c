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

#include "keyboard.h"

#include <xcb/xtest.h>

#include "utils.h"

// the global passive grab lets us replay keyboard events
#ifndef USE_XCB_GLOBAL_PASSIVE_GRAB
#define USE_XCB_GLOBAL_PASSIVE_GRAB (1)
#endif

typedef struct LastEvent
{
    xcb_time_t time;
    BackendKeyboardEvent event;
    BackendKeyboardEventResponse response;
} LastEvent;

static gboolean emulate_key(BackendXCBKeyboard *keyboard, guint keycode, gboolean pressed);

static void set_grab(BackendXCBKeyboard *keyboard);
static void unset_grab(BackendXCBKeyboard *keyboard);

static void set_key_grab(BackendXCBKeyboard *keyboard, BackendKeyboardEvent *grab);
static void unset_key_grab(BackendXCBKeyboard *keyboard, BackendKeyboardEvent *grab);

static void callback_key_event(xcb_generic_event_t *generic_event, gpointer keyboard_ptr);
static void callback_focus(gpointer keyboard_ptr);

#if USE_XCB_GLOBAL_PASSIVE_GRAB
static BackendKeyboardEvent GLOBAL_PASSIVE_GRAB = {
    .keycode = XCB_GRAB_ANY,
    .state.modifiers = XCB_GRAB_ANY,
    .state.group = XCB_GRAB_ANY,
};
#endif

// create a new keyboard listener
BackendXCBKeyboard *backend_xcb_keyboard_new(BackendXCB *backend, BackendKeyboardCallback callback, gpointer data)
{
    BackendXCBKeyboard *keyboard = g_new(BackendXCBKeyboard, 1);

    // add backend
    keyboard->backend = backend;

    // add callback
    keyboard->callback = callback;
    keyboard->data = data;

    // add x connection
    keyboard->connection = backend_xcb_get_connection(keyboard->backend);
    keyboard->root_window = backend_xcb_get_root_window(keyboard->backend);

    // get device ids
    keyboard->keyboard_id = backend_xcb_device_id_from_device_type(keyboard->connection, XCB_INPUT_DEVICE_TYPE_MASTER_KEYBOARD);
    keyboard->pointer_id = backend_xcb_device_id_from_device_type(keyboard->connection, XCB_INPUT_DEVICE_TYPE_MASTER_POINTER);

    // initialize grabs
    keyboard->grabs = 0;
    keyboard->key_grabs = NULL;

    // add focus listener
    keyboard->focus = backend_xcb_focus_new(keyboard->backend, callback_focus, keyboard);
    keyboard->grab_window = backend_xcb_focus_get_xcb_window(keyboard->focus);

    // init last events
    keyboard->last_keys = g_hash_table_new_full(NULL, NULL, NULL, g_free);

    // init emulated events
    keyboard->emulated_keys = g_hash_table_new(NULL, NULL);

    // subscribe to key events
    backend_xcb_subscribe(keyboard->backend, BACKEND_XCB_EXTENSION_XINPUT, XCB_INPUT_KEY_PRESS, callback_key_event, keyboard);
    backend_xcb_subscribe(keyboard->backend, BACKEND_XCB_EXTENSION_XINPUT, XCB_INPUT_KEY_RELEASE, callback_key_event, keyboard);

    return keyboard;
}

// destroy the keyboard listener
void backend_xcb_keyboard_destroy(BackendXCBKeyboard *keyboard)
{
    // unsubscribe from key events
    backend_xcb_unsubscribe(keyboard->backend, BACKEND_XCB_EXTENSION_XINPUT, XCB_INPUT_KEY_PRESS, callback_key_event, keyboard);
    backend_xcb_unsubscribe(keyboard->backend, BACKEND_XCB_EXTENSION_XINPUT, XCB_INPUT_KEY_RELEASE, callback_key_event, keyboard);

    // free last events
    g_hash_table_unref(keyboard->last_keys);

    // free any emulation
    backend_xcb_keyboard_emulate_reset(keyboard);
    g_hash_table_unref(keyboard->emulated_keys);

    // free
    g_free(keyboard);
}

// grab all keyboard input
void backend_xcb_keyboard_grab(BackendXCBKeyboard *keyboard)
{
    // add a grab
    keyboard->grabs++;

    // ensure grab is applied
    set_grab(keyboard);
}

// ungrab all keyboard input
void backend_xcb_keyboard_ungrab(BackendXCBKeyboard *keyboard)
{
    // check if the grab exists
    if (keyboard->grabs == 0)
        return;

    // remove a grab
    keyboard->grabs--;

    // remove the grab if none exist
    if (keyboard->grabs == 0)
        unset_grab(keyboard);
}

// grab input of a specific key
void backend_xcb_keyboard_grab_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event)
{
    // allocate grab
    BackendKeyboardEvent *grab = g_new(BackendKeyboardEvent, 1);
    *grab = event;

    // add grab
    keyboard->key_grabs = g_list_append(keyboard->key_grabs, grab);

    // ensure the grab is set
    set_key_grab(keyboard, grab);
}

// ungrab input of a specific key
void backend_xcb_keyboard_ungrab_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event)
{
    // remove the first instance of the grab
    for (GList *link = keyboard->key_grabs; link; link = link->next)
    {
        BackendKeyboardEvent *grab = link->data;

        // check if grab matches
        if (!((grab->keycode == event.keycode) &&
              (grab->state.modifiers == event.state.modifiers) &&
              (grab->state.group == event.state.group)))
            continue;

        // remove grab
        keyboard->key_grabs = g_list_delete_link(keyboard->key_grabs, link);

        // check if grab is duplicated
        gboolean unique_grab = FALSE;
        for (GList *link = keyboard->key_grabs; link; link = link->next)
        {
            BackendKeyboardEvent *found_grab = link->data;

            // check if grab matches
            if (!((grab->keycode == found_grab->keycode) &&
                  (grab->state.modifiers == found_grab->state.modifiers) &&
                  (grab->state.group == found_grab->state.group)))
                continue;

            // set unique
            unique_grab = FALSE;
            break;
        }

        // unset grab if it is unique
        if (unique_grab)
            unset_key_grab(keyboard, grab);

        // free grab
        g_free(grab);
        return;
    }
}

// get keyboard state
BackendKeyboardState backend_xcb_keyboard_get_state(BackendXCBKeyboard *keyboard)
{
    // send request
    xcb_input_xi_query_pointer_cookie_t cookie;
    cookie = xcb_input_xi_query_pointer(keyboard->connection, keyboard->root_window, keyboard->pointer_id);

    // get response
    xcb_generic_error_t *error = NULL;
    xcb_input_xi_query_pointer_reply_t *reply;
    reply = xcb_input_xi_query_pointer_reply(keyboard->connection, cookie, &error);
    if (error != NULL)
    {
        g_warning("backend-xcb: Failed to query pointer: error: %d", error->error_code);
        free(error);
    }
    if (!reply)
    {
        BackendKeyboardState state = {
            .modifiers = 0,
            .group = 0,
        };
        return state;
    }

    // get state
    // xcb has a bug where the effective modifiers are not computed here
    BackendKeyboardState state = {
        .modifiers = reply->mods.base | reply->mods.latched | reply->mods.locked | reply->mods.effective,
        .group = reply->group.base | reply->group.latched | reply->group.locked | reply->group.effective,
    };
    free(reply);
    return state;
}

// reset any emulated keys or state
gboolean backend_xcb_keyboard_emulate_reset(BackendXCBKeyboard *keyboard)
{
    // get lists of keys to send
    GList *keys_to_press = NULL;
    GList *keys_to_release = NULL;
    GHashTableIter iter;
    gpointer keycode, pressed;
    g_hash_table_iter_init(&iter, keyboard->emulated_keys);
    while (g_hash_table_iter_next(&iter, &keycode, &pressed))
        if (GPOINTER_TO_UINT(pressed))
            keys_to_release = g_list_prepend(keys_to_release, keycode);
        else
            keys_to_press = g_list_prepend(keys_to_press, keycode);

    // send keys
    gboolean is_success = TRUE;
    for (GList *link = keys_to_press; link; link = link->next)
        is_success &= emulate_key(keyboard, GPOINTER_TO_UINT(link->data), TRUE);
    g_list_free(keys_to_press);
    for (GList *link = keys_to_release; link; link = link->next)
        is_success &= emulate_key(keyboard, GPOINTER_TO_UINT(link->data), FALSE);
    g_list_free(keys_to_release);

    // return
    return is_success;
}

// set an emulated state
gboolean backend_xcb_keyboard_emulate_state(BackendXCBKeyboard *keyboard, BackendKeyboardState state)
{
    // todo: group

    // get current state
    BackendKeyboardState current_state = backend_xcb_keyboard_get_state(keyboard);

    // send get modifiers mapping request
    xcb_input_get_device_modifier_mapping_cookie_t cookie;
    cookie = xcb_input_get_device_modifier_mapping(keyboard->connection, keyboard->keyboard_id);

    // get the reply
    xcb_generic_error_t *error = NULL;
    xcb_input_get_device_modifier_mapping_reply_t *reply;
    reply = xcb_input_get_device_modifier_mapping_reply(keyboard->connection, cookie, &error);
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
            emulate_key(keyboard, keycode, press);

            // check if was set
            current_state = backend_xcb_keyboard_get_state(keyboard);
            if ((state.modifiers & (1 << mod_index)) == (current_state.modifiers & (1 << mod_index)))
                break;
            else
                emulate_key(keyboard, keycode, !press);
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
gboolean backend_xcb_keyboard_emulate_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event)
{
    // set the state
    if (!backend_xcb_keyboard_emulate_state(keyboard, event.state))
        return FALSE;

    // send the key event
    if (!emulate_key(keyboard, event.keycode, event.pressed))
        return FALSE;

    // return success
    return TRUE;
}

static gboolean emulate_key(BackendXCBKeyboard *keyboard, guint keycode, gboolean press)
{
    g_debug("backend-xcb: Emulating key: keycode: %d, press: %d", keycode, press);

    // send key
    xcb_void_cookie_t cookie;
    // todo: deviceid can apparently be 0, will that still get picked up by grabs?
    cookie = xcb_test_fake_input_checked(keyboard->connection,
                                         (press) ? XCB_KEY_PRESS : XCB_KEY_RELEASE,
                                         keycode,
                                         XCB_CURRENT_TIME,
                                         keyboard->root_window,
                                         0, 0,
                                         keyboard->keyboard_id);

    // check response
    xcb_generic_error_t *error = xcb_request_check(keyboard->connection, cookie);
    if (error)
    {
        g_warning("backend-xcb: Key emulation failed: keycode: %d, press: %d, error %d",
                  keycode, press, error->error_code);
        free(error);
        return FALSE;
    }

    // record in emulation table
    if (g_hash_table_contains(keyboard->emulated_keys, GUINT_TO_POINTER(keycode)))
    {
        if (GPOINTER_TO_UINT(g_hash_table_lookup(keyboard->emulated_keys, GUINT_TO_POINTER(keycode))) != press)
            g_hash_table_remove(keyboard->emulated_keys, GUINT_TO_POINTER(keycode));
    }
    else
    {
        g_hash_table_insert(keyboard->emulated_keys, GUINT_TO_POINTER(keycode), GUINT_TO_POINTER(press));
    }

    // return success
    return TRUE;
}

// apply the full device grab
static void set_grab(BackendXCBKeyboard *keyboard)
{
#if USE_XCB_GLOBAL_PASSIVE_GRAB
    set_key_grab(keyboard, &GLOBAL_PASSIVE_GRAB);
#else
    // send request
    const uint32_t mask[] = {XCB_INPUT_XI_EVENT_MASK_KEY_PRESS |
                             XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE};
    xcb_input_xi_grab_device_cookie_t cookie;
    cookie = xcb_input_xi_grab_device(keyboard->connection,
                                      keyboard->grab_window,
                                      XCB_CURRENT_TIME,
                                      XCB_NONE,
                                      keyboard->keyboard_id,
                                      XCB_INPUT_GRAB_MODE_22_SYNC,
                                      XCB_INPUT_GRAB_MODE_22_SYNC,
                                      FALSE,
                                      1,
                                      mask);

    // get response
    xcb_generic_error_t *error = NULL;
    xcb_input_xi_grab_device_reply_t *reply;
    reply = xcb_input_xi_grab_device_reply(keyboard->connection,
                                           cookie,
                                           &error);

    // handle response
    if (error != NULL)
    {
        g_warning("backend-xcb: Failed to grab device: error: %d", error->error_code);
        free(error);
    }
    if (reply)
        free(reply);
#endif
}

// remove the full device grab
static void unset_grab(BackendXCBKeyboard *keyboard)
{
#if USE_XCB_GLOBAL_PASSIVE_GRAB
    unset_key_grab(keyboard, &GLOBAL_PASSIVE_GRAB);
#else
    // send request
    xcb_void_cookie_t cookie;
    cookie = xcb_input_xi_ungrab_device_checked(keyboard->connection,
                                                XCB_CURRENT_TIME,
                                                keyboard->keyboard_id);

    // get response
    xcb_generic_error_t *error = xcb_request_check(keyboard->connection, cookie);

    // handle response
    if (error)
    {
        g_warning("backend-xcb: Failed to ungrab device: error: %d", error->error_code);
        free(error);
    }
#endif
}

// apply a passive key grab
static void set_key_grab(BackendXCBKeyboard *keyboard, BackendKeyboardEvent *grab)
{
    // send request
    // todo: add group
    const uint32_t modifiers[] = {grab->state.modifiers};
    const uint32_t mask[] = {XCB_INPUT_XI_EVENT_MASK_KEY_PRESS |
                             XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE};
    xcb_input_xi_passive_grab_device_cookie_t cookie;
    cookie = xcb_input_xi_passive_grab_device(keyboard->connection,
                                              XCB_CURRENT_TIME,
                                              keyboard->grab_window,
                                              XCB_NONE,
                                              grab->keycode,
                                              keyboard->keyboard_id,
                                              1,
                                              1,
                                              XCB_INPUT_GRAB_TYPE_KEYCODE,
                                              XCB_INPUT_GRAB_MODE_22_SYNC,
                                              XCB_INPUT_GRAB_MODE_22_SYNC,
                                              FALSE,
                                              mask,
                                              modifiers);

    // get response
    xcb_generic_error_t *error = NULL;
    xcb_input_xi_passive_grab_device_reply_t *reply = NULL;
    reply = xcb_input_xi_passive_grab_device_reply(keyboard->connection,
                                                   cookie,
                                                   &error);
    if (error)
    {
        g_warning("backend-xcb: Failed to grab key: keycode %d, modifiers: %d, error: %d",
                  grab->keycode, grab->state.modifiers, error->error_code);
        free(error);
    }
    if (reply)
        free(reply);
}

// remove a passive key grab
static void unset_key_grab(BackendXCBKeyboard *keyboard, BackendKeyboardEvent *grab)
{
    // send request
    // todo: add group
    const uint32_t modifiers[] = {grab->state.modifiers};
    xcb_void_cookie_t cookie;
    cookie = xcb_input_xi_passive_ungrab_device_checked(keyboard->connection,
                                                        keyboard->grab_window,
                                                        grab->keycode,
                                                        keyboard->keyboard_id,
                                                        1,
                                                        XCB_INPUT_GRAB_TYPE_KEYCODE,
                                                        modifiers);

    // get response
    xcb_generic_error_t *error = xcb_request_check(keyboard->connection, cookie);
    if (error)
    {
        g_warning("backend-xcb: Failed to ungrab key: keycode %d, modifiers: %d, error: %d",
                  grab->keycode, grab->state.modifiers, error->error_code);
        free(error);
    }
}

// callback for handling key events
static void callback_key_event(xcb_generic_event_t *generic_event, gpointer keyboard_ptr)
{
    BackendXCBKeyboard *keyboard = keyboard_ptr;

    // get key event
    xcb_input_key_press_event_t *key_event = (xcb_input_key_press_event_t *)generic_event;

    // get event data
    BackendKeyboardEvent event;
    event.keycode = key_event->detail;
    event.pressed = (key_event->event_type == XCB_INPUT_KEY_PRESS);
    event.state.modifiers = key_event->mods.effective;
    event.state.group = key_event->group.effective;

    // get last event of this key type
    LastEvent *last_event = g_hash_table_lookup(keyboard->last_keys, GUINT_TO_POINTER(event.keycode));
    if (last_event == NULL)
    {
        last_event = g_new0(LastEvent, 1);
        g_hash_table_insert(keyboard->last_keys, GUINT_TO_POINTER(event.keycode), last_event);
    }

    // check if the event is a duplicate
    gboolean duplicate = (key_event->time == last_event->time &&
                          event.keycode == last_event->event.keycode &&
                          event.pressed == last_event->event.pressed &&
                          event.state.modifiers == last_event->event.state.modifiers &&
                          event.state.group == last_event->event.state.group);

    // only send the event if it is not a duplicate
    BackendKeyboardEventResponse response;
    if (!duplicate)
        response = keyboard->callback(event, keyboard->data);
    else
        response = last_event->response;

    // save this event
    last_event->time = key_event->time;
    last_event->event = event;
    last_event->response = response;

    g_message("got key: press: %d, detail: %d, mods: %d, duplicate %d, consuming: %d",
              key_event->event_type == XCB_INPUT_KEY_PRESS, key_event->detail, key_event->mods.effective,
              duplicate, response == BACKEND_KEYBOARD_EVENT_CONSUME);

    // consume or relay the event
    guint8 event_mode = (response == BACKEND_KEYBOARD_EVENT_CONSUME) ? XCB_INPUT_EVENT_MODE_ASYNC_DEVICE
                                                                     : XCB_INPUT_EVENT_MODE_REPLAY_DEVICE;
    xcb_void_cookie_t cookie = xcb_input_xi_allow_events_checked(keyboard->connection,
                                                                 key_event->time,
                                                                 key_event->deviceid,
                                                                 event_mode,
                                                                 0,
                                                                 keyboard->root_window);
    xcb_generic_error_t *error = xcb_request_check(keyboard->connection, cookie);
    if (error)
    {
        g_warning("backend-xcb: Allow events failed: error %d", error->error_code);
        free(error);
    }
    xcb_flush(keyboard->connection);
}

static void callback_focus(gpointer keyboard_ptr)
{
    BackendXCBKeyboard *keyboard = keyboard_ptr;

    // get the focused window for the grab window
    // by setting the grab window to the focused window, focus out events are not sent
    keyboard->grab_window = backend_xcb_focus_get_xcb_window(keyboard->focus);
    if (keyboard->grab_window == XCB_NONE)
        keyboard->grab_window = keyboard->root_window;

    // set the global grab to the new window
    if (keyboard->grabs > 0)
        set_grab(keyboard);

    // set the key grabs to the new window
    for (GList *link = keyboard->key_grabs; link; link = link->next)
        set_key_grab(keyboard, link->data);
}

#endif /* USE_XCB */
