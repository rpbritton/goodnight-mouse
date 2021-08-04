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

#include "keyboard.h"

#include "modifiers.h"

typedef struct Subscriber
{
    KeyboardCallback callback;
    gpointer data;
    gboolean all_events;
    KeyboardEvent event;
} Subscriber;

// callback to handle an atspi keyboard event
static gboolean callback_atspi(AtspiDeviceEvent *atspi_event, gpointer listener_ptr);
static KeyboardResponse notify_subscribers(KeyboardListener *listener, KeyboardEvent event);
static gint compare_subscriber_to_callback(gconstpointer subscriber_ptr, gconstpointer callback_ptr);

// creates a new keyboard event listener and starts listening
KeyboardListener *keyboard_listener_new()
{
    KeyboardListener *listener = g_new(KeyboardListener, 1);

    // init subscribers
    listener->subscribers = NULL;

    // register listener
    listener->device_listener = atspi_device_listener_new(callback_atspi, listener, NULL);
    for (gint modifiers = 0; modifiers < 0xFF; modifiers++)
        atspi_register_keystroke_listener(listener->device_listener,
                                          NULL,
                                          modifiers,
                                          KEYBOARD_EVENT_PRESSED | KEYBOARD_EVENT_RELEASED,
                                          ATSPI_KEYLISTENER_SYNCHRONOUS | ATSPI_KEYLISTENER_CANCONSUME,
                                          NULL);

    return listener;
}

// stops and destroys a keyboard listener
void keyboard_listener_destroy(KeyboardListener *listener)
{
    // deregister listener
    for (gint modifiers = 0; modifiers < 0xFF; modifiers++)
        atspi_deregister_keystroke_listener(listener->device_listener,
                                            NULL,
                                            modifiers,
                                            KEYBOARD_EVENT_PRESSED | KEYBOARD_EVENT_RELEASED,
                                            NULL);
    g_object_unref(listener->device_listener);

    // free subscribers
    g_list_free_full(listener->subscribers, g_free);

    // free
    g_free(listener);
}

// subscribe a callback to all keyboard events
void keyboard_listener_subscribe(KeyboardListener *listener, KeyboardCallback callback, gpointer data)
{
    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;
    subscriber->all_events = TRUE;

    // add subscriber
    listener->subscribers = g_list_append(listener->subscribers, subscriber);
}

// subscribe a callback to a particular keyboard event
void keyboard_listener_subscribe_key(KeyboardListener *listener, KeyboardEvent event, KeyboardCallback callback, gpointer data)
{
    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;
    subscriber->all_events = FALSE;
    subscriber->event.key = event.key;
    subscriber->event.type = event.type;
    subscriber->event.modifiers = keyboard_modifiers_map(event.modifiers);

    // add subscriber
    listener->subscribers = g_list_append(listener->subscribers, subscriber);
}

// remove a callback from the subscribers
void keyboard_listener_unsubscribe(KeyboardListener *listener, KeyboardCallback callback)
{
    // find every instance of the callback and remove
    GList *link = NULL;
    while ((link = g_list_find_custom(listener->subscribers, callback, compare_subscriber_to_callback)))
        listener->subscribers = g_list_delete_link(listener->subscribers, link);
}

// callback to handle an atspi keyboard event
static gboolean callback_atspi(AtspiDeviceEvent *atspi_event, gpointer listener_ptr)
{
    // get event
    KeyboardEvent event = keyboard_event_from_atspi(atspi_event);
    g_boxed_free(ATSPI_TYPE_DEVICE_EVENT, atspi_event);

    // notify subscribers
    return notify_subscribers(listener_ptr, event);
}

// send an event to subscribers
static KeyboardResponse notify_subscribers(KeyboardListener *listener, KeyboardEvent event)
{
    // it only will take one subscriber to consume the event
    KeyboardResponse response = KEYBOARD_EVENT_RELAY;
    for (GList *link = listener->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if event matches against subscription
        if (!subscriber->all_events &&
            (subscriber->event.key != event.key ||
             (subscriber->event.type & event.type) == 0 ||
             subscriber->event.modifiers != event.modifiers))
            continue;

        // notify subscriber
        if (subscriber->callback(event, subscriber->data) == KEYBOARD_EVENT_CONSUME)
            response = KEYBOARD_EVENT_CONSUME;
    }

    return response;
}

// check if a subscriber matches a callback, returning 0 if so
static gint compare_subscriber_to_callback(gconstpointer subscriber_ptr, gconstpointer callback_ptr)
{
    return !(((Subscriber *)subscriber_ptr)->callback == callback_ptr);
}
