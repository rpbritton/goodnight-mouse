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

// subscriber of keyboard events
typedef struct Subscriber
{
    KeyboardCallback callback;
    gpointer data;
    gboolean all_events;
    KeyboardEvent event;
} Subscriber;

// callback to handle an atspi keyboard event
static void callback_keyboard(BackendKeyboardEvent backend_event, gpointer keyboard_ptr);

// creates a new keyboard event keyboard and starts listening
Keyboard *keyboard_new(Backend *backend)
{
    Keyboard *keyboard = g_new(Keyboard, 1);

    // init subscribers
    keyboard->subscribers = NULL;

    // add backends
    keyboard->keyboard = backend_keyboard_new(backend, callback_keyboard, keyboard);
    keyboard->modifiers = backend_modifiers_new(backend);

    return keyboard;
}

// stops and destroys a keyboard keyboard
void keyboard_destroy(Keyboard *keyboard)
{
    // free backends
    backend_keyboard_destroy(keyboard->keyboard);
    backend_modifiers_destroy(keyboard->modifiers);

    // free subscribers
    g_list_free_full(keyboard->subscribers, g_free);

    // free
    g_free(keyboard);
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

    // grab the keyboard
    backend_keyboard_grab(keyboard->keyboard);
}

// remove keyboard event subscription
void keyboard_unsubscribe(Keyboard *keyboard, KeyboardCallback callback, gpointer data)
{
    // remove the first matching subscriber
    for (GList *link = keyboard->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if subscriber matches
        if (!((subscriber->callback == callback) &&
              (subscriber->data == data) &&
              (subscriber->all_events == TRUE)))
            continue;

        // remove subscriber
        g_free(subscriber);
        keyboard->subscribers = g_list_delete_link(keyboard->subscribers, link);

        // ungrab the keyboard
        backend_keyboard_ungrab(keyboard->keyboard);

        return;
    }
}

// subscribe a callback to a particular keyboard event
void keyboard_subscribe_key(Keyboard *keyboard, KeyboardEvent event, KeyboardCallback callback, gpointer data)
{
    // sanitize modifier input
    event.modifiers = keyboard_modifiers_map(keyboard, event.modifiers);

    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;
    subscriber->all_events = FALSE;
    subscriber->event = event;

    // add subscriber
    keyboard->subscribers = g_list_append(keyboard->subscribers, subscriber);

    // grab the key
    BackendKeyboardEvent backend_event = {
        .keysym = event.keysym,
        .type = ((event.type & KEYBOARD_EVENT_PRESSED) ? BACKEND_KEYBOARD_EVENT_PRESSED : 0) |
                ((event.type & KEYBOARD_EVENT_RELEASED) ? BACKEND_KEYBOARD_EVENT_RELEASED : 0),
        .modifiers = event.modifiers,
    };
    backend_keyboard_grab_key(keyboard->keyboard, backend_event);
}

// removes a key subscription
void keyboard_unsubscribe_key(Keyboard *keyboard, KeyboardEvent event, KeyboardCallback callback, gpointer data)
{
    // sanitize modifier input
    event.modifiers = keyboard_modifiers_map(keyboard, event.modifiers);

    // remove the first matching subscriber
    for (GList *link = keyboard->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if subscriber matches
        if (!((subscriber->callback == callback) &&
              (subscriber->data == data) &&
              (subscriber->all_events == FALSE) &&
              (subscriber->event.keysym == event.keysym) &&
              (subscriber->event.type == event.type) &&
              (subscriber->event.modifiers == event.modifiers)))
            continue;

        // remove subscriber
        g_free(subscriber);
        keyboard->subscribers = g_list_delete_link(keyboard->subscribers, link);

        // ungrab the key// map event
        BackendKeyboardEvent backend_event = {
            .keysym = event.keysym,
            .type = (event.type == KEYBOARD_EVENT_PRESSED) ? BACKEND_KEYBOARD_EVENT_PRESSED
                                                           : BACKEND_KEYBOARD_EVENT_RELEASED,
            .modifiers = event.modifiers,
        };
        backend_keyboard_ungrab_key(keyboard->keyboard, backend_event);

        return;
    }
}

// callback to handle an atspi keyboard event
static void callback_keyboard(BackendKeyboardEvent backend_event, gpointer keyboard_ptr)
{
    Keyboard *keyboard = keyboard_ptr;

    // map event
    KeyboardEvent event = {
        .keysym = backend_event.keysym,
        .type = (backend_event.type == BACKEND_KEYBOARD_EVENT_PRESSED) ? KEYBOARD_EVENT_PRESSED
                                                                       : KEYBOARD_EVENT_RELEASED,
        .modifiers = keyboard_modifiers_map(keyboard, backend_event.modifiers),
    };

    // notify subscribers
    for (GList *link = keyboard->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if event matches against subscription
        if (!((subscriber->all_events) ||
              ((subscriber->event.keysym == event.keysym) &&
               (subscriber->event.type & event.type) &&
               (subscriber->event.modifiers == event.modifiers))))
            continue;

        // notify subscriber
        subscriber->callback(event, subscriber->data);
    }
}

// get the current modifiers
guint keyboard_modifiers_get(Keyboard *keyboard)
{
    return backend_modifiers_get(keyboard->modifiers);
}

// map the given modifiers
guint keyboard_modifiers_map(Keyboard *keyboard, guint mods)
{
    return backend_modifiers_map(keyboard->modifiers, mods);
}
