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

    // set root window events
    uint32_t values[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_change_window_attributes(backend->connection, backend->root,
                                 XCB_CW_EVENT_MASK, values);

    // select xcb events
    // todo: xcb_input_xi_select_events

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
void backend_xcb_subscribe(BackendXCB *backend, guint8 type, BackendXCBCallback callback, gpointer data)
{
    // create new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->type = type;
    subscriber->callback = callback;
    subscriber->data = data;

    // add the subscriber
    backend->subscribers = g_list_append(backend->subscribers, subscriber);
}

// unsubscribe from xcb events
void backend_xcb_unsubscribe(BackendXCB *backend, guint8 type, BackendXCBCallback callback, gpointer data)
{
    // remove the first matching subscriber
    for (GList *link = backend->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if subscriber matches
        if (!(subscriber->type == type &&
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

    g_message("got event of type %d", event->response_type);

    // notify subscribers
    for (GList *link = backend->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if subscriber matches
        if (!(subscriber->type == event->response_type))
            continue;

        // notify subscriber
        subscriber->callback(event, subscriber->data);
    }

    return G_SOURCE_CONTINUE;
}

#endif /* USE_XCB */
