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

#include "source.h"

static gboolean xcb_source_prepare(GSource *source, gint *timeout);
static gboolean xcb_source_check(GSource *source);
static gboolean xcb_source_dispatch(GSource *source, GSourceFunc callback, gpointer user_data);
static void xcb_source_finalize(GSource *source);

static GSourceFuncs xcb_source_funcs = {
    .prepare = xcb_source_prepare,
    .check = xcb_source_check,
    .dispatch = xcb_source_dispatch,
    .finalize = xcb_source_finalize,
};

typedef struct XCBSource
{
    GSource source;
    xcb_connection_t *connection;
    GPollFD poll_fd;
    GQueue *events;
} XCBSource;

// create a new xcb event loop source
GSource *xcb_source_new(xcb_connection_t *connection)
{
    // create the source
    GSource *source = g_source_new(&xcb_source_funcs, sizeof(XCBSource));
    XCBSource *xcb_source = (XCBSource *)source;

    // add connection
    xcb_source->connection = connection;

    // add file descriptor polling
    xcb_source->poll_fd.fd = xcb_get_file_descriptor(connection);
    xcb_source->poll_fd.events = G_IO_IN;
    g_source_add_poll(source, &xcb_source->poll_fd);

    // create queue
    xcb_source->events = g_queue_new();

    // return
    return source;
}

// xcb source prepare
static gboolean xcb_source_prepare(GSource *source, gint *timeout)
{
    XCBSource *xcb_source = (XCBSource *)source;

    // timeout doesn't matter
    *timeout = -1;

    // flush xcb
    xcb_flush(xcb_source->connection);

    // return whether events exist
    return !g_queue_is_empty(xcb_source->events);
}

// xcb source check event loop
static gboolean xcb_source_check(GSource *source)
{
    XCBSource *xcb_source = (XCBSource *)source;

    // check the file descriptor
    if (xcb_source->poll_fd.revents & G_IO_IN)
    {
        // read all pending events
        xcb_generic_event_t *event;
        while ((event = xcb_poll_for_event(xcb_source->connection)))
            g_queue_push_tail(xcb_source->events, event);
    }

    // return whether events are pending
    return !g_queue_is_empty(xcb_source->events);
}

// xcb source process event loop
static gboolean xcb_source_dispatch(GSource *source, GSourceFunc callback, gpointer data)
{
    XCBSource *xcb_source = (XCBSource *)source;

    // do nothing if no callback
    if (!callback)
        return G_SOURCE_CONTINUE;

    // get the event
    xcb_generic_event_t *event = g_queue_pop_head(xcb_source->events);

    // call the callback
    gboolean state = ((XCBSourceCallback)callback)(event, data);

    // free event
    free(event);

    // return
    return state;
}

// xcb source free
static void xcb_source_finalize(GSource *source)
{
    XCBSource *xcb_source = (XCBSource *)source;

    // free queue
    g_queue_free_full(xcb_source->events, free);
}

#endif /* USE_XCB */
