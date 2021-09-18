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

#include "device.h"

// the global passive grab lets us replay device events
#ifndef USE_XCB_GLOBAL_PASSIVE_GRAB
#define USE_XCB_GLOBAL_PASSIVE_GRAB (1)
#endif

// record used to detect duplicate events
typedef struct LastDetailEvent
{
    xcb_time_t time;
    uint16_t event_type;
    BackendXCBDeviceEventResponse response;
} LastDetailEvent;

// structure for defining a detail grab
typedef struct BackendXCBDetailGrab
{
    guint32 detail;
    guint32 modifiers;
} BackendXCBDetailGrab;

static void set_device_grab(BackendXCBDevice *device);
static void unset_device_grab(BackendXCBDevice *device);

static void set_detail_grab(BackendXCBDevice *device, BackendXCBDetailGrab *grab);
static void unset_detail_grab(BackendXCBDevice *device, BackendXCBDetailGrab *grab);

static void callback_xcb(xcb_generic_event_t *event, gpointer device_ptr);
static void callback_focus(gpointer device_ptr);

#if USE_XCB_GLOBAL_PASSIVE_GRAB
static BackendXCBDetailGrab GLOBAL_PASSIVE_GRAB = {
    .detail = XCB_GRAB_ANY,
    .modifiers = XCB_GRAB_ANY,
};
#endif

// create a new device listener
BackendXCBDevice *backend_xcb_device_new(BackendXCB *backend, xcb_input_device_id_t device_id,
                                         guint32 event_mask, BackendXCBDeviceCallback callback, gpointer data)
{
    BackendXCBDevice *device = g_new(BackendXCBDevice, 1);

    // add backend
    device->backend = backend;

    // add device info
    device->device_id = device_id;
    device->event_mask = event_mask;

    // add callback
    device->callback = callback;
    device->data = data;

    // add x connection
    device->connection = backend_xcb_get_connection(device->backend);
    device->root_window = backend_xcb_get_root_window(device->backend);

    // initialize grabs
    device->device_grabs = 0;
    device->detail_grabs = NULL;

    // add focus listener
    device->focus = backend_xcb_focus_new(device->backend, callback_focus, device);
    device->grab_window = backend_xcb_focus_get_xcb_window(device->focus);

    // init last events
    device->last_events = g_hash_table_new_full(NULL, NULL, NULL, g_free);

    // subscribe to the events
    backend_xcb_subscribe(device->backend,
                          BACKEND_XCB_EXTENSION_XINPUT, device->event_mask,
                          callback_xcb, device);

    return device;
}

// destroy the device listener
void backend_xcb_device_destroy(BackendXCBDevice *device)
{
    // unsubscribe from detail events
    backend_xcb_unsubscribe(device->backend,
                            BACKEND_XCB_EXTENSION_XINPUT, device->event_mask,
                            callback_xcb, device);

    // free last events
    g_hash_table_unref(device->last_events);

    // free
    g_free(device);
}

// grab all device input
void backend_xcb_device_grab(BackendXCBDevice *device)
{
    // add a grab
    device->device_grabs++;

    // ensure grab is applied if new
    if (device->device_grabs == 1)
        set_device_grab(device);
}

// ungrab all device input
void backend_xcb_device_ungrab(BackendXCBDevice *device)
{
    // check if the grab exists
    if (device->device_grabs == 0)
        return;

    // remove a grab
    device->device_grabs--;

    // remove the grab if none exist
    if (device->device_grabs == 0)
        unset_device_grab(device);
}

// grab input of a specific detail
void backend_xcb_device_grab_detail(BackendXCBDevice *device, guint32 detail, guint32 modifiers)
{
    // allocate grab
    BackendXCBDetailGrab *grab = g_new(BackendXCBDetailGrab, 1);
    grab->detail = detail;
    grab->modifiers = modifiers;

    // add grab
    device->detail_grabs = g_list_append(device->detail_grabs, grab);

    // ensure the grab is set
    set_detail_grab(device, grab);
}

// ungrab input of a specific detail
void backend_xcb_device_ungrab_detail(BackendXCBDevice *device, guint32 detail, guint32 modifiers)
{
    // remove the first instance of the grab
    for (GList *link = device->detail_grabs; link; link = link->next)
    {
        BackendXCBDetailGrab *grab = link->data;

        // check if grab matches
        if (!((grab->detail == detail) &&
              (grab->modifiers == modifiers)))
            continue;

        // remove grab
        device->detail_grabs = g_list_delete_link(device->detail_grabs, link);

        // check if grab is duplicated
        gboolean unique_grab = FALSE;
        for (GList *link = device->detail_grabs; link; link = link->next)
        {
            BackendXCBDetailGrab *found_grab = link->data;

            // check if grab matches
            if (!((found_grab->detail == grab->detail) &&
                  (found_grab->modifiers == grab->modifiers)))
                continue;

            // set unique
            unique_grab = FALSE;
            break;
        }

        // unset grab if it is unique
        if (unique_grab)
            unset_detail_grab(device, grab);

        // free grab
        g_free(grab);
        break;
    }
}

// apply the full device grab
static void set_device_grab(BackendXCBDevice *device)
{
#if USE_XCB_GLOBAL_PASSIVE_GRAB
    set_detail_grab(device, &GLOBAL_PASSIVE_GRAB);
#else
    // send request
    xcb_input_xi_grab_device_cookie_t cookie;
    cookie = xcb_input_xi_grab_device(device->connection,
                                      device->grab_window,
                                      XCB_CURRENT_TIME,
                                      XCB_NONE,
                                      device->device_id,
                                      XCB_INPUT_GRAB_MODE_22_ASYNC,
                                      XCB_INPUT_GRAB_MODE_22_ASYNC,
                                      FALSE,
                                      1,
                                      &device->event_mask);

    // get response
    xcb_generic_error_t *error = NULL;
    xcb_input_xi_grab_device_reply_t *reply;
    reply = xcb_input_xi_grab_device_reply(device->connection,
                                           cookie,
                                           &error);
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
static void unset_device_grab(BackendXCBDevice *device)
{
#if USE_XCB_GLOBAL_PASSIVE_GRAB
    unset_detail_grab(device, &GLOBAL_PASSIVE_GRAB);
#else
    // send request
    xcb_void_cookie_t cookie;
    cookie = xcb_input_xi_ungrab_device_checked(device->connection,
                                                XCB_CURRENT_TIME,
                                                device->device_id);

    // get response
    xcb_generic_error_t *error = xcb_request_check(device->connection, cookie);
    if (error)
    {
        g_warning("backend-xcb: Failed to ungrab device: error: %d", error->error_code);
        free(error);
    }
#endif
}

// apply a passive detail grab
static void set_detail_grab(BackendXCBDevice *device, BackendXCBDetailGrab *grab)
{
    // send request
    xcb_input_xi_passive_grab_device_cookie_t cookie;
    cookie = xcb_input_xi_passive_grab_device(device->connection,
                                              XCB_CURRENT_TIME,
                                              device->grab_window,
                                              XCB_NONE,
                                              grab->detail,
                                              device->device_id,
                                              1,
                                              1,
                                              XCB_INPUT_GRAB_TYPE_KEYCODE,
                                              XCB_INPUT_GRAB_MODE_22_SYNC,
                                              XCB_INPUT_GRAB_MODE_22_SYNC,
                                              FALSE,
                                              &device->event_mask,
                                              &grab->modifiers);

    // get response
    xcb_generic_error_t *error = NULL;
    xcb_input_xi_passive_grab_device_reply_t *reply = NULL;
    reply = xcb_input_xi_passive_grab_device_reply(device->connection, cookie, &error);
    if (error)
    {
        g_warning("backend-xcb: Failed to grab detail: detail %d, modifiers: %d, error: %d",
                  grab->detail, grab->modifiers, error->error_code);
        free(error);
    }
    if (reply)
        free(reply);
}

// remove a passive detail grab
static void unset_detail_grab(BackendXCBDevice *device, BackendXCBDetailGrab *grab)
{
    // send request
    xcb_void_cookie_t cookie;
    cookie = xcb_input_xi_passive_ungrab_device_checked(device->connection,
                                                        device->grab_window,
                                                        grab->detail,
                                                        device->device_id,
                                                        1,
                                                        XCB_INPUT_GRAB_TYPE_KEYCODE,
                                                        &grab->modifiers);

    // get response
    xcb_generic_error_t *error = xcb_request_check(device->connection, cookie);
    if (error)
    {
        g_warning("backend-xcb: Failed to ungrab detail: detail %d, modifiers: %d, error: %d",
                  grab->detail, grab->modifiers, error->error_code);
        free(error);
    }
}

// callback for handling xinput events
static void callback_xcb(xcb_generic_event_t *generic_event, gpointer device_ptr)
{
    BackendXCBDevice *device = device_ptr;

    // get device event
    xcb_input_key_press_event_t *event = (xcb_input_key_press_event_t *)generic_event;

    // get last event of this detail type
    LastDetailEvent *last_event = g_hash_table_lookup(device->last_events, GUINT_TO_POINTER(event->detail));
    if (last_event == NULL)
    {
        last_event = g_new0(LastDetailEvent, 1);
        g_hash_table_insert(device->last_events, GUINT_TO_POINTER(event->detail), last_event);
    }

    // check if the event is a duplicate
    gboolean is_duplicate = (event->time == last_event->time &&
                             event->event_type == last_event->event_type);

    // only send the event if it is not a duplicate
    BackendXCBDeviceEventResponse response;
    if (!is_duplicate)
        response = device->callback(generic_event, device->data);
    else
        response = last_event->response;

    // save this event
    last_event->time = event->time;
    last_event->event_type = event->event_type;
    last_event->response = response;

    g_message("got event: event_type: %d, detail: %d, mods: %d, is_duplicate %d, consuming: %d",
              event->event_type, event->detail, event->mods.effective,
              is_duplicate, response == BACKEND_XCB_DEVICE_EVENT_CONSUME);

    // consume or relay the event
    guint8 event_mode = (response == BACKEND_XCB_DEVICE_EVENT_CONSUME) ? XCB_INPUT_EVENT_MODE_ASYNC_DEVICE
                                                                       : XCB_INPUT_EVENT_MODE_REPLAY_DEVICE;
    xcb_void_cookie_t cookie = xcb_input_xi_allow_events_checked(device->connection,
                                                                 event->time,
                                                                 event->deviceid,
                                                                 event_mode,
                                                                 0,
                                                                 device->root_window);
    xcb_generic_error_t *error = xcb_request_check(device->connection, cookie);
    if (error)
    {
        g_warning("backend-xcb: Allow events failed: error %d", error->error_code);
        free(error);
    }
    // todo: needed?
    xcb_flush(device->connection);
}

// listen for focus events
// by setting the grab window to the focused window, focus stays with that window
static void callback_focus(gpointer device_ptr)
{
    BackendXCBDevice *device = device_ptr;

    // get the focused window for the grab window
    xcb_window_t grab_window = backend_xcb_focus_get_xcb_window(device->focus);
    if (grab_window == XCB_NONE)
        grab_window = device->root_window;

    // ensure grab window is different
    if (grab_window == device->grab_window)
        return;

    // unset the global grab
    if (device->device_grabs > 0)
        unset_device_grab(device);

    // unset the detail grabs
    for (GList *link = device->detail_grabs; link; link = link->next)
        unset_detail_grab(device, link->data);

    // set the new grab window
    device->grab_window = grab_window;

    // reset the global grab
    if (device->device_grabs > 0)
        set_device_grab(device);

    // reset the detail grabs
    for (GList *link = device->detail_grabs; link; link = link->next)
        set_detail_grab(device, link->data);
}

#endif /* USE_XCB */
