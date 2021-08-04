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

#include "mouse.h"

typedef struct Subscriber
{
    MouseCallback callback;
    gpointer data;
} Subscriber;

static gboolean callback_atspi(AtspiDeviceEvent *atspi_event, gpointer listener_ptr);
static MouseResponse notify_subscribers(Mouse *listener, MouseEvent event);
static gint compare_subscriber_to_callback(gconstpointer subscriber_ptr, gconstpointer callback_ptr);

// creates a new mouse event listener and starts listening
Mouse *mouse_new()
{
    Mouse *listener = g_new(Mouse, 1);

    // init subscribers
    listener->subscribers = NULL;

    // register listener
    listener->atspi_listener = atspi_device_listener_new(callback_atspi, listener, NULL);
    atspi_register_device_event_listener(listener->atspi_listener,
                                         MOUSE_EVENT_PRESSED | MOUSE_EVENT_RELEASED,
                                         NULL,
                                         NULL);

    return listener;
}

// stops and destroys a mouse listener
void mouse_destroy(Mouse *listener)
{
    // deregister listener
    atspi_deregister_device_event_listener(listener->atspi_listener, NULL, NULL);
    g_object_unref(listener->atspi_listener);

    // free subscribers
    g_list_free_full(listener->subscribers, g_free);

    // free
    g_free(listener);
}

// subscribe a callback to mouse events
void mouse_subscribe(Mouse *listener, MouseCallback callback, gpointer data)
{
    // don't add if subscribed
    if (g_list_find_custom(listener->subscribers, callback, compare_subscriber_to_callback))
        return;

    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;

    // add subscriber
    listener->subscribers = g_list_append(listener->subscribers, subscriber);
}

// remove a callback from the subscribers
void mouse_unsubscribe(Mouse *listener, MouseCallback callback)
{
    // find every instance of the callback and remove
    GList *link = NULL;
    while ((link = g_list_find_custom(listener->subscribers, callback, compare_subscriber_to_callback)))
        listener->subscribers = g_list_delete_link(listener->subscribers, link);
}

// callback to handle an atspi mouse event
static gboolean callback_atspi(AtspiDeviceEvent *atspi_event, gpointer listener_ptr)
{
    // get event
    MouseEvent event = mouse_event_from_atspi(atspi_event);
    g_boxed_free(ATSPI_TYPE_DEVICE_EVENT, atspi_event);

    // notify subscribers
    return notify_subscribers(listener_ptr, event);
}

// send an event to all subscribers
static MouseResponse notify_subscribers(Mouse *listener, MouseEvent event)
{
    // it only will take one subscriber to consume the event
    MouseResponse response = MOUSE_EVENT_RELAY;
    for (GList *link = listener->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;
        if (subscriber->callback(event, subscriber->data) == MOUSE_EVENT_CONSUME)
            response = MOUSE_EVENT_CONSUME;
    }

    return response;
}

// check if a subscriber matches a callback, returning 0 if so
static gint compare_subscriber_to_callback(gconstpointer subscriber_ptr, gconstpointer callback_ptr)
{
    return !(((Subscriber *)subscriber_ptr)->callback == callback_ptr);
}
