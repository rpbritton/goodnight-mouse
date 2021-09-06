/**
 * Copyright (C) 2021 ryan
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

#if USE_X11

#include "backend.h"

#include <X11/extensions/XInput2.h>

#include "source.h"

static gboolean process_events(gpointer backend_ptr);

BackendX11 *backend_x11_new()
{
    BackendX11 *backend = g_new(BackendX11, 1);

    // open x connection
    backend->display = XOpenDisplay(NULL);

    // select x11 events
    XSelectInput(backend->display, XDefaultRootWindow(backend->display), PropertyChangeMask);

    // select xinput events
    unsigned char events[XIMaskLen(XI_LASTEVENT)] = {0};
    XISetMask(events, XI_KeyPress);
    XISetMask(events, XI_KeyRelease);
    XISetMask(events, XI_ButtonPress);
    XISetMask(events, XI_ButtonRelease);

    XIEventMask event_mask = {
        .deviceid = XIAllDevices,
        .mask_len = sizeof(events),
        .mask = events,
    };
    XISelectEvents(backend->display, XDefaultRootWindow(backend->display), &event_mask, 1);

    // add the x11 source
    backend->source = x11_source_new(backend->display);
    g_source_set_callback(backend->source, process_events, backend, NULL);
    g_source_attach(backend->source, NULL);

    // return
    return backend;
}

void backend_x11_destroy(BackendX11 *backend)
{
    // close display
    XCloseDisplay(backend->display);

    // free
    g_free(backend);
}

Display *backend_x11_get_display(BackendX11 *backend)
{
    return backend->display;
}

static gboolean process_events(gpointer backend_ptr)
{
    BackendX11 *backend = backend_ptr;

    // process all the events
    while (XPending(backend->display))
    {
        XEvent event;
        XNextEvent(backend->display, &event);
        g_message("whoo got event %d", event.type);
        switch (event.type)
        {
        default:
            //XFilterEvent() todo?
            break;
        }
    }

    return G_SOURCE_CONTINUE;
}

#endif /* USE_X11 */
