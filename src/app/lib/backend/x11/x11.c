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

#include "x11.h"

#include <X11/extensions/XInput2.h>
#include <X11/XKBlib.h>

#include "source.h"

typedef struct Subscriber
{
    BackendX11Extension extension;
    int type;
    BackendX11Callback callback;
    gpointer data;
} Subscriber;

static void setup_xinput(BackendX11 *backend);
static void setup_xkb(BackendX11 *backend);
static gboolean process_events(gpointer backend_ptr);
static void proccess_extension_event(BackendX11 *backend, XEvent *event);
static void notify_subscribers(BackendX11 *backend, XEvent *event,
                               BackendX11Extension extension, int type);

BackendX11 *backend_x11_new()
{
    BackendX11 *backend = g_new(BackendX11, 1);

    // open x connection
    backend->display = XOpenDisplay(NULL);

    // select x11 events
    XSelectInput(backend->display, XDefaultRootWindow(backend->display), PropertyChangeMask);

    // setup extensions
    setup_xinput(backend);
    setup_xkb(backend);

    // add the x11 source
    backend->source = x11_source_new(backend->display);
    g_source_set_callback(backend->source, process_events, backend, NULL);
    g_source_attach(backend->source, NULL);

    // create legacy backend
    backend->legacy = backend_legacy_new();

    // init the subscribers
    backend->subscribers = NULL;

    // return
    return backend;
}

void backend_x11_destroy(BackendX11 *backend)
{
    // close display
    XCloseDisplay(backend->display);

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

void backend_x11_subscribe(BackendX11 *backend, BackendX11Extension extension, int type, BackendX11Callback callback, gpointer data)
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

void backend_x11_unsubscribe(BackendX11 *backend, BackendX11Extension extension, int type, BackendX11Callback callback, gpointer data)
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

Display *backend_x11_get_display(BackendX11 *backend)
{
    return backend->display;
}

BackendLegacy *backend_x11_get_legacy(BackendX11 *backend)
{
    return backend->legacy;
}

static void setup_xinput(BackendX11 *backend)
{
    // get xinput opcode
    int first_event, first_error;
    if (!XQueryExtension(backend->display, "XInputExtension", &backend->xinput_opcode,
                         &first_event, &first_error))
        g_error("x11: XInput not available");

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
}

static void setup_xkb(BackendX11 *backend)
{
    // get xkb opcode
    int first_event, first_error, major_in, major_out;
    if (!XkbQueryExtension(backend->display, &backend->xkb_opcode, &first_event, &first_error, &major_in, &major_out))
        g_error("x11: XKB not available");

    // select xkb events
    XkbSelectEvents(backend->display, XkbUseCoreKbd, XkbMapNotify, 1);
}

static gboolean process_events(gpointer backend_ptr)
{
    BackendX11 *backend = backend_ptr;

    // process all the events
    while (XPending(backend->display))
    {
        // get the event
        XEvent event;
        XNextEvent(backend->display, &event);

        // process event
        if (event.type == GenericEvent)
            proccess_extension_event(backend, &event);
        else
            notify_subscribers(backend, &event, BACKEND_X11_EVENT_TYPE_X11, event.type);
    }

    return G_SOURCE_CONTINUE;
}

static void proccess_extension_event(BackendX11 *backend, XEvent *event)
{
    // get extension type
    BackendX11Extension extension;
    if (event->xcookie.extension == backend->xinput_opcode)
        extension = BACKEND_X11_EVENT_TYPE_XINPUT;
    else if (event->xcookie.extension == backend->xkb_opcode)
        extension = BACKEND_X11_EVENT_TYPE_XKB;
    else
        return;

    g_message("processing event");

    // get event data
    XGetEventData(backend->display, &event->xcookie);

    // notify subscribers
    notify_subscribers(backend, event, extension, event->xcookie.evtype);

    // free
    XFreeEventData(backend->display, &event->xcookie);
}

static void notify_subscribers(BackendX11 *backend, XEvent *event,
                               BackendX11Extension extension, int type)
{
    // notify subscribers
    for (GList *link = backend->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if subscriber matches
        if (!(subscriber->extension == extension &&
              subscriber->type == type))
            continue;

        // notify subscriber
        subscriber->callback(event, subscriber->data);
    }
}

#endif /* USE_X11 */
