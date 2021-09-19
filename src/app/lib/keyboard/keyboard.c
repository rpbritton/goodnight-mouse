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
    gboolean all_keys;
    guint keysym;
    GdkModifierType modifiers;
    GList *grabs;
} Subscriber;

static BackendKeyboardEventResponse callback_keyboard(BackendKeyboardEvent backend_event, gpointer keyboard_ptr);

// creates a new keyboard event keyboard and starts listening
Keyboard *keyboard_new(Backend *backend, Keymap *keymap)
{
    Keyboard *keyboard = g_new(Keyboard, 1);

    // add backend
    keyboard->backend = backend_keyboard_new(backend, callback_keyboard, keyboard);

    // add keymap
    keyboard->keymap = keymap;

    // init subscribers
    keyboard->subscribers = NULL;

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
    subscriber->all_keys = TRUE;

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
              (subscriber->all_keys == TRUE)))
            continue;

        // ungrab the keyboard
        backend_keyboard_ungrab(keyboard->backend);

        // remove subscriber
        keyboard->subscribers = g_list_delete_link(keyboard->subscribers, link);
        g_free(subscriber);
        return;
    }
}

// subscribe a callback to a particular keyboard event
void keyboard_subscribe_key(Keyboard *keyboard, guint keysym, GdkModifierType modifiers, KeyboardCallback callback, gpointer data)
{
    // sanitize modifiers
    modifiers = keymap_physical_modifiers(keyboard->keymap, modifiers);

    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;
    subscriber->all_keys = FALSE;
    subscriber->keysym = keysym;
    subscriber->modifiers = modifiers;
    subscriber->grabs = keymap_get_keycodes(keyboard->keymap, keysym, modifiers);

    // add the key grabs
    for (GList *link = subscriber->grabs; link; link = link->next)
    {
        BackendKeyboardEvent *grab = link->data;
        g_debug("keyboard: Grabbing key: keycode: %d, modifiers: 0x%X", grab->keycode, grab->state.modifiers);
        backend_keyboard_grab_key(keyboard->backend, grab->keycode, grab->state);
    }

    // note if grabs were found
    if (!subscriber->grabs)
        g_warning("keyboard: No valid keys to grab: keysym: %d, modifiers: 0x%X", keysym, modifiers);

    // add subscriber
    keyboard->subscribers = g_list_append(keyboard->subscribers, subscriber);
}

// removes a key subscription
void keyboard_unsubscribe_key(Keyboard *keyboard, guint keysym, GdkModifierType modifiers, KeyboardCallback callback, gpointer data)
{
    // sanitize modifiers
    modifiers = keymap_physical_modifiers(keyboard->keymap, modifiers);

    // remove the first matching subscriber
    for (GList *link = keyboard->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if subscriber matches
        if (!((subscriber->callback == callback) &&
              (subscriber->data == data) &&
              (subscriber->all_keys == FALSE) &&
              (subscriber->keysym == keysym) &&
              (subscriber->modifiers == modifiers)))
            continue;

        // ungrab the keys
        for (GList *link = subscriber->grabs; link; link = link->next)
        {
            BackendKeyboardEvent *grab = link->data;
            backend_keyboard_ungrab_key(keyboard->backend, grab->keycode, grab->state);
        }
        g_list_free_full(subscriber->grabs, g_free);

        // remove subscriber
        keyboard->subscribers = g_list_delete_link(keyboard->subscribers, link);
        g_free(subscriber);

        return;
    }
}

// callback to handle an atspi keyboard event
static BackendKeyboardEventResponse callback_keyboard(BackendKeyboardEvent backend_event, gpointer keyboard_ptr)
{
    Keyboard *keyboard = keyboard_ptr;

    // parse event
    KeyboardEvent event;
    guint8 hotkey_modifiers;
    event.keysym = keymap_get_keysym(keyboard->keymap, backend_event, &hotkey_modifiers);
    event.pressed = backend_event.pressed;
    event.modifiers = keymap_all_modifiers(keyboard->keymap, backend_event.state.modifiers);

    // notify subscribers
    KeyboardEventResponse response = BACKEND_KEYBOARD_EVENT_RELAY;
    for (GList *link = keyboard->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if event matches against subscription
        if (!((subscriber->all_keys) ||
              ((subscriber->keysym == event.keysym) &&
               (subscriber->modifiers == hotkey_modifiers))))
            continue;

        // notify subscriber
        if (subscriber->callback(event, subscriber->data) == KEYBOARD_EVENT_CONSUME)
            response = BACKEND_KEYBOARD_EVENT_CONSUME;
    }

    // return response
    return response;
}
