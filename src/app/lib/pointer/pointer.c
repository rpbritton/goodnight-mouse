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

#include "pointer.h"

// subscriber of pointer events
typedef struct Subscriber
{
    PointerCallback callback;
    gpointer data;
    gboolean all_buttons;
    guint button;
    guint8 modifiers;
} Subscriber;

static BackendPointerEventResponse callback_pointer(BackendPointerEvent backend_event, gpointer pointer_ptr);

// creates a new pointer and starts listening
Pointer *pointer_new(Backend *backend, Keymap *keymap)
{
    Pointer *pointer = g_new(Pointer, 1);

    // add backend
    pointer->backend = backend_pointer_new(backend, callback_pointer, pointer);

    // add keymap
    pointer->keymap = keymap;

    // init subscribers
    pointer->subscribers = NULL;

    return pointer;
}

// stops and destroys a pointer
void pointer_destroy(Pointer *pointer)
{
    // free backend
    backend_pointer_destroy(pointer->backend);

    // free subscribers
    g_list_free_full(pointer->subscribers, g_free);

    // free
    g_free(pointer);
}

// subscribe a callback to all pointer events
void pointer_subscribe(Pointer *pointer, PointerCallback callback, gpointer data)
{
    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;
    subscriber->all_buttons = TRUE;

    // add subscriber
    pointer->subscribers = g_list_append(pointer->subscribers, subscriber);

    // grab the pointer
    backend_pointer_grab(pointer->backend);
}

// remove pointer event subscription
void pointer_unsubscribe(Pointer *pointer, PointerCallback callback, gpointer data)
{
    // remove the first matching subscriber
    for (GList *link = pointer->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if subscriber matches
        if (!((subscriber->callback == callback) &&
              (subscriber->data == data) &&
              (subscriber->all_buttons == TRUE)))
            continue;

        // ungrab the pointer
        backend_pointer_ungrab(pointer->backend);

        // remove subscriber
        pointer->subscribers = g_list_delete_link(pointer->subscribers, link);
        g_free(subscriber);
        return;
    }
}

// subscribe a callback to a particular pointer event
void pointer_subscribe_button(Pointer *pointer, guint button, GdkModifierType modifiers, PointerCallback callback, gpointer data)
{
    // sanitize modifiers
    modifiers = keymap_physical_modifiers(pointer->keymap, modifiers);

    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;
    subscriber->all_buttons = FALSE;
    subscriber->button = button;
    subscriber->modifiers = modifiers;

    // add the button grab
    g_debug("pointer: Grabbing button: button: %d, modifiers: 0x%X", subscriber->button, subscriber->modifiers);
    // todo: set group
    BackendStateEvent state = {
        .modifiers = subscriber->modifiers,
        .group = 0,
    };
    backend_pointer_grab_button(pointer->backend, subscriber->button, state);

    // add subscriber
    pointer->subscribers = g_list_append(pointer->subscribers, subscriber);
}

// removes a key subscription
void pointer_unsubscribe_button(Pointer *pointer, guint button, GdkModifierType modifiers, PointerCallback callback, gpointer data)
{
    // sanitize modifiers
    modifiers = keymap_physical_modifiers(pointer->keymap, modifiers);

    // remove the first matching subscriber
    for (GList *link = pointer->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if subscriber matches
        if (!((subscriber->callback == callback) &&
              (subscriber->data == data) &&
              (subscriber->all_buttons == FALSE) &&
              (subscriber->button == button) &&
              (subscriber->modifiers == modifiers)))
            continue;

        // ungrab the keys
        // todo: set group
        BackendStateEvent state = {
            .modifiers = subscriber->modifiers,
            .group = 0,
        };
        backend_pointer_ungrab_button(pointer->backend, subscriber->button, state);

        // remove subscriber
        pointer->subscribers = g_list_delete_link(pointer->subscribers, link);
        g_free(subscriber);
        return;
    }
}

// callback to handle an atspi pointer event
static BackendPointerEventResponse callback_pointer(BackendPointerEvent backend_event, gpointer pointer_ptr)
{
    Pointer *pointer = pointer_ptr;

    // parse event
    PointerEvent event;
    event.button = backend_event.button;
    event.pressed = backend_event.pressed;
    event.modifiers = keymap_all_modifiers(pointer->keymap, backend_event.state.modifiers);

    // get only hotkey modifiers
    guint8 hotkey_modifiers = keymap_hotkey_modifiers(pointer->keymap, backend_event.state.modifiers);

    // notify subscribers
    BackendPointerEventResponse response = BACKEND_POINTER_EVENT_RELAY;
    for (GList *link = pointer->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if event matches against subscription
        if (!((subscriber->all_buttons) ||
              ((subscriber->button == event.button) &&
               (subscriber->modifiers == hotkey_modifiers))))
            continue;

        // notify subscriber
        if (subscriber->callback(event, subscriber->data) == POINTER_EVENT_CONSUME)
            response = BACKEND_POINTER_EVENT_CONSUME;
    }

    // return response
    return response;
}
