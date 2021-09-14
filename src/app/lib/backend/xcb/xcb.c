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

#include "xcb.h"

#include "source.h"

// subscriber to xcb events
typedef struct Subscriber
{
    BackendXCBExtension extension;
    guint8 type;
    BackendXCBCallback callback;
    gpointer data;
} Subscriber;

static gboolean process_event(xcb_generic_event_t *event, gpointer backend_ptr);

// create a new backend using xcb
BackendXCB *backend_xcb_new()
{
    BackendXCB *backend = g_new(BackendXCB, 1);

    // open x connection
    backend->connection = xcb_connect(NULL, NULL);
    const xcb_setup_t *setup = xcb_get_setup(backend->connection);
    xcb_screen_t *screen = xcb_setup_roots_iterator(setup).data;
    backend->root = screen->root;

    // set xcb events
    uint32_t values[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_change_window_attributes(backend->connection, backend->root, XCB_CW_EVENT_MASK, values);

    // get xinput op code
    const xcb_query_extension_reply_t *xinput_reply = xcb_get_extension_data(backend->connection, &xcb_input_id);
    if (!xinput_reply || !xinput_reply->present)
        g_error("backend-xcb: XInputExtension not found");
    backend->extension_xinput = xinput_reply->major_opcode;
    // todo: need you free xinput_reply?

    // select xinput events
    struct
    {
        xcb_input_event_mask_t head;
        xcb_input_xi_event_mask_t mask;
    } xinput_mask;
    xinput_mask.head.deviceid = XCB_INPUT_DEVICE_ALL;
    // xinput_mask.head.deviceid = XCB_INPUT_DEVICE_ALL_MASTER;
    xinput_mask.head.mask_len = sizeof(xinput_mask.mask) / sizeof(uint32_t);
    xinput_mask.mask = XCB_INPUT_XI_EVENT_MASK_KEY_PRESS | XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE;
    //                   XCB_INPUT_XI_EVENT_MASK_RAW_KEY_PRESS | XCB_INPUT_XI_EVENT_MASK_RAW_KEY_RELEASE;
    xcb_input_xi_select_events(backend->connection, backend->root, 1, &xinput_mask.head);

    // add the event source
    backend->source = xcb_source_new(backend->connection);
    g_source_set_callback(backend->source, G_SOURCE_FUNC(process_event), backend, NULL);
    g_source_attach(backend->source, NULL);

    // create legacy backend
    backend->legacy = backend_legacy_new();

    // init the subscribers
    backend->subscribers = NULL;

    // return
    return backend;
}

// destroy the backend
void backend_xcb_destroy(BackendXCB *backend)
{
    // close connection
    xcb_disconnect(backend->connection);

    // remove source
    g_source_destroy(backend->source);
    g_source_unref(backend->source);

    // free legacy backend
    backend_legacy_destroy(backend->legacy);

    // free subscribers
    g_list_free_full(backend->subscribers, g_free);

    // free
    g_free(backend);
}

// subscribe to xcb events
void backend_xcb_subscribe(BackendXCB *backend, BackendXCBExtension extension, guint8 type, BackendXCBCallback callback, gpointer data)
{
    // create new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->extension = extension;
    subscriber->type = type;
    subscriber->callback = callback;
    subscriber->data = data;

    // add the subscriber
    backend->subscribers = g_list_append(backend->subscribers, subscriber);
}

// unsubscribe from xcb events
void backend_xcb_unsubscribe(BackendXCB *backend, BackendXCBExtension extension, guint8 type, BackendXCBCallback callback, gpointer data)
{
    // remove the first matching subscriber
    for (GList *link = backend->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if subscriber matches
        if (!(subscriber->extension == extension &&
              subscriber->type == type &&
              subscriber->callback == callback &&
              subscriber->data == data))
            continue;

        // remove subscriber
        backend->subscribers = g_list_delete_link(backend->subscribers, link);
        g_free(subscriber);
        return;
    }
}

// get the xcb connection
xcb_connection_t *backend_xcb_get_connection(BackendXCB *backend)
{
    return backend->connection;
}

// get the xcb root window
xcb_window_t backend_xcb_get_root(BackendXCB *backend)
{
    return backend->root;
}

// get the legacy fallback backend
BackendLegacy *backend_xcb_get_legacy(BackendXCB *backend)
{
    return backend->legacy;
}

// send an xcb event to the subscribers
static gboolean process_event(xcb_generic_event_t *event, gpointer backend_ptr)
{
    BackendXCB *backend = backend_ptr;

    // get event extension and type
    BackendXCBExtension extension = BACKEND_XCB_EXTENSION_NONE;
    guint8 type = event->response_type;
    if (type == XCB_GE_GENERIC)
    {
        xcb_ge_generic_event_t *ge_event = (xcb_ge_generic_event_t *)event;
        if (ge_event->extension == backend->extension_xinput)
        {
            extension = BACKEND_XCB_EXTENSION_XINPUT;
            type = ge_event->event_type;
        }
    }
    g_message("got event: extension: %d, type: %d", extension, type);

    // notify subscribers
    for (GList *link = backend->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if subscriber matches
        // todo: do you need to do (event->response_type & ~0x80)?
        if (!(subscriber->extension == extension &&
              subscriber->type == type))
            continue;

        // notify subscriber
        subscriber->callback(event, subscriber->data);
    }

    return G_SOURCE_CONTINUE;
}

#endif /* USE_XCB */
