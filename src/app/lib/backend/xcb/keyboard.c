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

#include "utils.h"

static void set_grab(BackendXCBKeyboard *keyboard);
static void unset_grab(BackendXCBKeyboard *keyboard);

static void set_key_grab(BackendXCBKeyboard *keyboard, BackendKeyboardEvent *grab);
static void unset_key_grab(BackendXCBKeyboard *keyboard, BackendKeyboardEvent *grab);

static void callback_key_event(xcb_generic_event_t *generic_event, gpointer keyboard_ptr);
static void callback_focus(gpointer keyboard_ptr);

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
    keyboard->keyboard_id = backend_xcb_device_id_from_device_type(keyboard->connection, XCB_INPUT_DEVICE_TYPE_MASTER_KEYBOARD);

    // initialize grabs
    keyboard->grabs = 0;
    keyboard->key_grabs = NULL;

    // add focus listener
    keyboard->focus = backend_xcb_focus_new(keyboard->backend, callback_focus, keyboard);
    keyboard->grab_window = backend_xcb_focus_get_xcb_window(keyboard->focus);

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
    g_message("todo: backend_xcb_keyboard_get_state");

    BackendKeyboardState state = {
        .group = 0,
        .modifiers = 0,
    };
    return state;
}

// reset any emulated keys or state
void backend_xcb_keyboard_emulate_reset(BackendXCBKeyboard *keyboard)
{
    g_message("todo: backend_xcb_keyboard_emulate_reset");
}

// set an emulated state
void backend_xcb_keyboard_emulate_state(BackendXCBKeyboard *keyboard, BackendKeyboardState state)
{
    g_message("todo: backend_xcb_keyboard_emulate_state");
}

// set an emulated key
void backend_xcb_keyboard_emulate_key(BackendXCBKeyboard *keyboard, BackendKeyboardEvent event)
{
    g_message("todo: backend_xcb_keyboard_emulate_key");
}

// apply the full device grab
static void set_grab(BackendXCBKeyboard *keyboard)
{
    // // send request
    // const uint32_t mask[] = {XCB_INPUT_XI_EVENT_MASK_KEY_PRESS |
    //                          XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE};
    // xcb_input_xi_grab_device_cookie_t cookie;
    // cookie = xcb_input_xi_grab_device(keyboard->connection,
    //                                   keyboard->root_window,
    //                                   XCB_CURRENT_TIME,
    //                                   XCB_NONE,
    //                                   keyboard->keyboard_id,
    //                                   XCB_INPUT_GRAB_MODE_22_ASYNC,
    //                                   XCB_INPUT_GRAB_MODE_22_ASYNC,
    //                                   FALSE,
    //                                   1,
    //                                   mask);

    // // get response
    // xcb_generic_error_t *error = NULL;
    // xcb_input_xi_grab_device_reply_t *reply;
    // reply = xcb_input_xi_grab_device_reply(keyboard->connection,
    //                                        cookie,
    //                                        &error);

    // // handle response
    // if (error != NULL)
    // {
    //     g_warning("backend-xcb: Failed to grab device: error: %d", error->error_code);
    //     free(error);
    // }
    // if (reply)
    //     free(reply);

    BackendKeyboardEvent grab;
    grab.keycode = 0;
    grab.state.modifiers = 0;
    grab.state.group = 0;

    set_key_grab(keyboard, &grab);
}

// remove the full device grab
static void unset_grab(BackendXCBKeyboard *keyboard)
{
    // // send request
    // xcb_void_cookie_t cookie;
    // cookie = xcb_input_xi_ungrab_device_checked(keyboard->connection,
    //                                             XCB_CURRENT_TIME,
    //                                             keyboard->keyboard_id);

    // // get response
    // xcb_generic_error_t *error = xcb_request_check(keyboard->connection, cookie);

    // // handle response
    // if (error)
    // {
    //     g_warning("backend-xcb: Failed to ungrab device: error: %d", error->error_code);
    //     free(error);
    // }

    BackendKeyboardEvent grab;
    grab.keycode = 0;
    grab.state.modifiers = 0;
    grab.state.group = 0;

    unset_key_grab(keyboard, &grab);
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

    // handle response
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

    // handle response
    if (error)
    {
        g_warning("backend-xcb: Failed to ungrab key: keycode %d, modifiers: %d, error: %d",
                  grab->keycode, grab->state.modifiers, error->error_code);
        free(error);
    }
}

// static gint counter = 0;

// callback for handling key events
static void callback_key_event(xcb_generic_event_t *generic_event, gpointer keyboard_ptr)
{
    BackendXCBKeyboard *keyboard = keyboard_ptr;

    // get key event
    xcb_input_key_press_event_t *key_event = (xcb_input_key_press_event_t *)generic_event;
    g_message("got key: press: %d, detail: %d, mods: %d",
              key_event->event_type == XCB_INPUT_KEY_PRESS, key_event->detail, key_event->mods.effective);

    // // testing
    // if (event->event_type == XCB_INPUT_KEY_PRESS)
    // {
    //     g_message("incrementing");
    //     counter++;
    // }

    // guint8 event_mode;
    // if (counter % 2 == 1)
    // {
    //     g_message("replaying");
    //     event_mode = XCB_INPUT_EVENT_MODE_REPLAY_DEVICE;
    // }
    // else
    // {
    //     g_message("asyncing");
    //     event_mode = XCB_INPUT_EVENT_MODE_ASYNC_DEVICE;
    // }
    // xcb_void_cookie_t cookie = xcb_input_xi_allow_events_checked(keyboard->connection,
    //                                                              event->time,
    //                                                              event->deviceid,
    //                                                              event_mode,
    //                                                              0,
    //                                                              keyboard->root_window);
    // xcb_generic_error_t *error = xcb_request_check(keyboard->connection, cookie);
    // if (error)
    // {
    //     g_warning("backend-xcb: Allow events failed: response: %d, error %d", error->response_type, error->error_code);
    //     free(error);
    // }

    // get event data
    BackendKeyboardEvent event;
    event.keycode = key_event->detail;
    event.pressed = (key_event->event_type == XCB_INPUT_KEY_PRESS);
    event.state.modifiers = key_event->mods.effective;
    event.state.group = key_event->group.effective;

    // // check for duplicate event
    // if (key_event->time == keyboard->last_event_time &&
    //     event.keycode == keyboard->last_event.keycode &&
    //     event.pressed == keyboard->last_event.pressed &&
    //     event.state.modifiers == keyboard->last_event.state.modifiers &&
    //     event.state.group == keyboard->last_event.state.group)
    //     return;
    // keyboard->last_event_time = key_event->time;
    // keyboard->last_event = event;

    // callback the event
    BackendKeyboardEventResponse response = keyboard->callback(event, keyboard->data);

    // consume or relay the event
    guint8 event_mode;
    if (response == BACKEND_KEYBOARD_EVENT_CONSUME)
    {
        g_message("consuming");
        event_mode = XCB_INPUT_EVENT_MODE_ASYNC_DEVICE;
    }
    else
    {
        g_message("replaying");
        event_mode = XCB_INPUT_EVENT_MODE_REPLAY_DEVICE;
    }
    xcb_void_cookie_t cookie = xcb_input_xi_allow_events_checked(keyboard->connection,
                                                                 key_event->time,
                                                                 key_event->deviceid,
                                                                 event_mode,
                                                                 0,
                                                                 keyboard->root_window);
    xcb_generic_error_t *error = xcb_request_check(keyboard->connection, cookie);
    if (error)
    {
        g_warning("backend-xcb: Allow events failed: response: %d, error %d", error->response_type, error->error_code);
        free(error);
    }
}

static void callback_focus(gpointer keyboard_ptr)
{
    BackendXCBKeyboard *keyboard = keyboard_ptr;

    // set the new active window to grab
    // this prevents the window from losing focus during key grabbing
    keyboard->grab_window = backend_xcb_focus_get_xcb_window(keyboard->focus);

    // set the global grab to the new window
    if (keyboard->grabs > 0)
        set_grab(keyboard);

    // set the key grabs to the new window
    for (GList *link = keyboard->key_grabs; link; link = link->next)
        set_key_grab(keyboard, link->data);
}

#endif /* USE_XCB */
