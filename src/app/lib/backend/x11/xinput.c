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

// static int handler(Display *display, XErrorEvent *error)
// {
//     g_message("WOW got an error %d", error->error_code);
//     return 0;
// }

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

    // XSetErrorHandler(handler);
    // XIDeviceEvent *device_event = x11_event->xcookie.data;

    // int deviceid;
    // g_message("prior deviceid: %d", deviceid);
    // Bool value = XIGetClientPointer(xinput->display, None, &deviceid);
    // g_message("value %d, deviceid %d", value, deviceid);

    // Window root, child;
    // double root_x, root_y, win_x, win_y;
    // XIButtonState buttons;
    // XIModifierState mods;
    // XIGroupState group;
    // g_message("prior mods %d", mods.effective);
    // XIQueryPointer(xinput->display, deviceid, XDefaultRootWindow(xinput->display), &root, &child, &root_x, &root_y, &win_x, &win_y, &buttons, &mods, &group);
    // g_message("value: %d, event mods: %d, pointer mods %d", value, device_event->mods.effective, mods.effective);

    // free
    XFreeEventData(xinput->display, &x11_event->xcookie);
}

#endif /* USE_X11 */
