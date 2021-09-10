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
static void callback_keyboard(KeyboardEvent event, gpointer keyboard_ptr);

// creates a new keyboard event keyboard and starts listening
Keyboard *keyboard_new(Backend *backend)
{
    Keyboard *keyboard = g_new(Keyboard, 1);

    // init subscribers
    keyboard->subscribers = NULL;

    // get keymap
    keyboard->keymap = gdk_keymap_get_for_display(gdk_display_get_default());

    // add backend
    keyboard->backend = backend_keyboard_new(backend, callback_keyboard, keyboard);

    return keyboard;
}

// stops and destroys a keyboard keyboard
void keyboard_destroy(Keyboard *keyboard)
{
    // free backend
    backend_keyboard_destroy(keyboard->backend);

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
    backend_keyboard_grab(keyboard->backend);
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
        backend_keyboard_ungrab(keyboard->backend);

        return;
    }
}

// subscribe a callback to a particular keyboard event
void keyboard_subscribe_key(Keyboard *keyboard, KeyboardEvent event, KeyboardCallback callback, gpointer data)
{
    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;
    subscriber->all_events = FALSE;
    subscriber->event = event;

    // add subscriber
    keyboard->subscribers = g_list_append(keyboard->subscribers, subscriber);

    // grab the key
    backend_keyboard_grab_key(keyboard->backend, event);
}

// removes a key subscription
void keyboard_unsubscribe_key(Keyboard *keyboard, KeyboardEvent event, KeyboardCallback callback, gpointer data)
{
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

        // ungrab the key
        backend_keyboard_ungrab_key(keyboard->backend, event);

        return;
    }
}

// callback to handle an atspi keyboard event
static void callback_keyboard(KeyboardEvent event, gpointer keyboard_ptr)
{
    Keyboard *keyboard = keyboard_ptr;

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
Modifiers keyboard_get_modifiers(Keyboard *keyboard)
{
    return backend_keyboard_get_modifiers(keyboard->backend);
}

// map virtual gdk modifiers into 8-bits
Modifiers keyboard_map_modifiers(Keyboard *keyboard, GdkModifierType mods)
{
    gdk_keymap_map_virtual_modifiers(keyboard->keymap, &mods);
    return mods & 0xFF;
}