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

#if USE_X11

#include "source.h"

static gboolean x11_source_prepare(GSource *source, gint *timeout);
static gboolean x11_source_check(GSource *source);
static gboolean x11_source_dispatch(GSource *source, GSourceFunc callback, gpointer user_data);

static GSourceFuncs x11_source_funcs = {
    .prepare = x11_source_prepare,
    .check = x11_source_check,
    .dispatch = x11_source_dispatch,
    .finalize = NULL,
};

typedef struct X11Source
{
    GSource source;
    Display *display;
    GPollFD poll_fd;
} X11Source;

GSource *x11_source_new(Display *display)
{
    // create the source
    GSource *source = g_source_new(&x11_source_funcs, sizeof(X11Source));
    X11Source *x11_source = (X11Source *)source;

    // add display
    x11_source->display = display;

    // add file descriptor polling
    x11_source->poll_fd.fd = ConnectionNumber(display);
    x11_source->poll_fd.events = G_IO_IN;
    g_source_add_poll(source, &x11_source->poll_fd);

    // return
    return source;
}

static gboolean x11_source_prepare(GSource *source, gint *timeout)
{
    X11Source *x11_source = (X11Source *)source;

    // timeout doesn't matter
    *timeout = -1;

    // return whether events are pending
    return XPending(x11_source->display);
}

static gboolean x11_source_check(GSource *source)
{
    X11Source *x11_source = (X11Source *)source;

    // check the file descriptor
    if (!(x11_source->poll_fd.revents & G_IO_IN))
        return FALSE;

    // return whether events are pending
    return XPending(x11_source->display);
}

static gboolean x11_source_dispatch(GSource *source, GSourceFunc callback, gpointer data)
{
    // do nothing if no callback
    if (!callback)
        return G_SOURCE_CONTINUE;

    // call the callback
    callback(data);
    return G_SOURCE_CONTINUE;
}

#endif /* USE_X11 */
