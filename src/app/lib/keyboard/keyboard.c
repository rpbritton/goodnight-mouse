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
static KeyboardResponse notify_subscribers(Keyboard *keyboard, KeyboardEvent event);
static gint compare_subscriber_to_callback(gconstpointer subscriber_ptr, gconstpointer callback_ptr);

// creates a new keyboard event keyboard and starts listening
Keyboard *keyboard_new()
{
    Keyboard *keyboard = g_new(Keyboard, 1);

    // init subscribers
    keyboard->subscribers = NULL;

    // register keyboard
    keyboard->registered = FALSE;
    keyboard->device_listener = atspi_device_listener_new(callback_atspi, keyboard, NULL);
    keyboard_register(keyboard);

    return keyboard;
}

// stops and destroys a keyboard keyboard
void keyboard_destroy(Keyboard *keyboard)
{
    // deregister keyboard
    keyboard_deregister(keyboard);
    g_object_unref(keyboard->device_listener);

    // free subscribers
    g_list_free_full(keyboard->subscribers, g_free);

    // free
    g_free(keyboard);
}

// registers keyboard event listening
void keyboard_register(Keyboard *keyboard)
{
    // do nothing if registered
    if (keyboard->registered)
        return;

    // register to all keyboard events
    for (gint modifiers = 0; modifiers < 0xFF; modifiers++)
        atspi_register_keystroke_listener(keyboard->device_listener,
                                          NULL,
                                          modifiers,
                                          KEYBOARD_EVENT_PRESSED | KEYBOARD_EVENT_RELEASED,
                                          ATSPI_KEYLISTENER_SYNCHRONOUS | ATSPI_KEYLISTENER_CANCONSUME,
                                          NULL);
}

// unregisters keyboard event listening
void keyboard_deregister(Keyboard *keyboard)
{
    // do nothing if not registered
    if (!keyboard->registered)
        return;

    // unregister from keyboard events
    for (gint modifiers = 0; modifiers < 0xFF; modifiers++)
        atspi_deregister_keystroke_listener(keyboard->device_listener,
                                            NULL,
                                            modifiers,
                                            KEYBOARD_EVENT_PRESSED | KEYBOARD_EVENT_RELEASED,
                                            NULL);
}

// subscribe a callback to all keyboard events
void keyboard_subscribe(Keyboard *keyboard, KeyboardCallback callback, gpointer data)
{
    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;
    subscriber->all_events = TRUE;

    // add subscriber
    keyboard->subscribers = g_list_append(keyboard->subscribers, subscriber);
}

// subscribe a callback to a particular keyboard event
void keyboard_subscribe_key(Keyboard *keyboard, KeyboardEvent event, KeyboardCallback callback, gpointer data)
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
    keyboard->subscribers = g_list_append(keyboard->subscribers, subscriber);
}

// remove a callback from the subscribers
void keyboard_unsubscribe(Keyboard *keyboard, KeyboardCallback callback)
{
    // find every instance of the callback and remove
    GList *link = NULL;
    while ((link = g_list_find_custom(keyboard->subscribers, callback, compare_subscriber_to_callback)))
        keyboard->subscribers = g_list_delete_link(keyboard->subscribers, link);
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
static KeyboardResponse notify_subscribers(Keyboard *keyboard, KeyboardEvent event)
{
    // it only will take one subscriber to consume the event
    KeyboardResponse response = KEYBOARD_EVENT_RELAY;
    for (GList *link = keyboard->subscribers; link; link = link->next)
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
