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

#define VALID_MODIFIERS (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK | GDK_SUPER_MASK)

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

// callback to handle an atspi keyboard event
static void callback_keyboard(BackendKeyboardEvent backend_event, gpointer keyboard_ptr);

// creates a new keyboard event keyboard and starts listening
Keyboard *keyboard_new(Backend *backend)
{
    Keyboard *keyboard = g_new(Keyboard, 1);

    // init subscribers
    keyboard->subscribers = NULL;

    // add keymap
    keyboard->keymap = gdk_keymap_get_for_display(gdk_display_get_default());
    GdkModifierType virtual_valid_modifiers = VALID_MODIFIERS;
    gdk_keymap_map_virtual_modifiers(keyboard->keymap, &virtual_valid_modifiers);
    keyboard->valid_modifiers = virtual_valid_modifiers & 0xFF;

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
    gdk_keymap_map_virtual_modifiers(keyboard->keymap, &modifiers);
    modifiers = modifiers & 0xFF & keyboard->valid_modifiers;
    g_message("grabbing keysym: %d, modifiers: %d", keysym, modifiers);

    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;
    subscriber->all_keys = FALSE;
    subscriber->keysym = keysym;
    subscriber->modifiers = modifiers;
    subscriber->grabs = NULL;

    // create the key grabs
    GdkKeymapKey *keys;
    gint n_keys;
    gdk_keymap_get_entries_for_keyval(keyboard->keymap, keysym, &keys, &n_keys);
    for (gint index = 0; index < n_keys; index++)
    {
        for (guchar modifiers = 0; modifiers < 0xFF; modifiers++)
        {
            // get keysym and consumed modifiers from state
            guint keysym;
            GdkModifierType consumed_modifiers;
            gdk_keymap_translate_keyboard_state(keyboard->keymap, keys[index].keycode,
                                                modifiers, keys[index].group,
                                                &keysym, NULL, NULL, &consumed_modifiers);

            // ensure keysym is the same
            if (keysym != subscriber->keysym)
                continue;

            // ensure modifiers match
            if (subscriber->modifiers != (modifiers & ~consumed_modifiers & keyboard->valid_modifiers))
                continue;

            // add the grab
            BackendKeyboardEvent *grab = g_new(BackendKeyboardEvent, 1);
            grab->keycode = keys[index].keycode;
            grab->state.modifiers = modifiers;
            grab->state.group = keys[index].group;
            subscriber->grabs = g_list_append(subscriber->grabs, grab);
            backend_keyboard_grab_key(keyboard->backend, *grab);
        }
    }
    if (keys)
        g_free(keys);

    // ensure grabs were found
    if (!subscriber->grabs)
        g_warning("keyboard: Subscription has no keys: keysym: %d, modifiers: %d", keysym, modifiers);

    // add subscriber
    keyboard->subscribers = g_list_append(keyboard->subscribers, subscriber);
}

// removes a key subscription
void keyboard_unsubscribe_key(Keyboard *keyboard, guint keysym, GdkModifierType modifiers, KeyboardCallback callback, gpointer data)
{
    // sanitize modifiers
    gdk_keymap_map_virtual_modifiers(keyboard->keymap, &modifiers);
    modifiers &= 0xFF;

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
            backend_keyboard_ungrab_key(keyboard->backend, *grab);
        }
        g_list_free_full(subscriber->grabs, g_free);

        // remove subscriber
        keyboard->subscribers = g_list_delete_link(keyboard->subscribers, link);
        g_free(subscriber);

        return;
    }
}

// callback to handle an atspi keyboard event
static void callback_keyboard(BackendKeyboardEvent backend_event, gpointer keyboard_ptr)
{
    Keyboard *keyboard = keyboard_ptr;

    // parse event
    KeyboardEvent event;

    // get keysym
    guint keysym;
    GdkModifierType consumed_modifiers;
    gdk_keymap_translate_keyboard_state(keyboard->keymap, backend_event.keycode,
                                        backend_event.state.modifiers, backend_event.state.group,
                                        &keysym, NULL, NULL, &consumed_modifiers);
    event.keysym = keysym;

    // get pressed
    event.pressed = backend_event.pressed;

    // get modifiers
    event.modifiers = backend_event.state.modifiers;
    gdk_keymap_add_virtual_modifiers(keyboard->keymap, &event.modifiers);

    // get active modifiers that were not consumed to create the keyval
    guchar active_modifiers = backend_event.state.modifiers &
                              ~consumed_modifiers &
                              keyboard->valid_modifiers;

    // notify subscribers
    for (GList *link = keyboard->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if event matches against subscription
        if (!((subscriber->all_keys) ||
              ((subscriber->keysym == event.keysym) &&
               (subscriber->modifiers == active_modifiers))))
            continue;

        // notify subscriber
        subscriber->callback(event, subscriber->data);
    }
}

// get the current modifiers
GdkModifierType keyboard_get_modifiers(Keyboard *keyboard)
{
    // get the current modifiers
    BackendKeyboardState state = backend_keyboard_get_modifiers(keyboard->backend);

    // sanitize the modifiers
    GdkModifierType modifiers = state.modifiers;
    gdk_keymap_add_virtual_modifiers(keyboard->keymap, &modifiers);

    // return
    return modifiers;
}