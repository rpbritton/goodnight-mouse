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

#include "xinput.h"

static void callback_x11(XEvent *event, gpointer data);

// create an xinput event parser
BackendX11XInput *backend_x11_xinput_new(BackendX11 *backend, BackendX11XInputCallback callback, gpointer data)
{
    BackendX11XInput *xinput = g_new(BackendX11XInput, 1);

    // add backend
    xinput->backend = backend;

    // add callback
    xinput->callback = callback;
    xinput->data = data;

    // add display
    xinput->display = backend_x11_get_display(xinput->backend);

    // get xinput opcode
    int first_event;
    int first_error;
    if (!XQueryExtension(xinput->display, "XInputExtension", &xinput->opcode,
                         &first_event, &first_error))
        g_error("xinput not available");

    // subscribe to x11 events
    backend_x11_subscribe(xinput->backend, GenericEvent, callback_x11, xinput);

    // return
    return xinput;
}

// destroy the xinput parser
void backend_x11_xinput_destroy(BackendX11XInput *xinput)
{
    // unsubscribe from x11 events
    backend_x11_unsubscribe(xinput->backend, GenericEvent, callback_x11, xinput);

    // free
    g_free(xinput);
}

static void callback_x11(XEvent *x11_event, gpointer xinput_ptr)
{
    BackendX11XInput *xinput = xinput_ptr;

    // only check xinput events
    if (x11_event->xcookie.extension != xinput->opcode)
        return;

    // get event data
    XGetEventData(xinput->display, &x11_event->xcookie);

    // callback subscriber
    xinput->callback(x11_event->xcookie.data, xinput->data);

    // free
    XFreeEventData(xinput->display, &x11_event->xcookie);
}

#endif /* USE_X11 */
