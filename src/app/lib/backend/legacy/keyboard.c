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

#include "keyboard.h"

// whether event should be passed on
typedef enum KeyboardEventResponse
{
    KEYBOARD_EVENT_RELAY = FALSE,
    KEYBOARD_EVENT_CONSUME = TRUE,
} KeyboardEventResponse;

static void listener_register(BackendLegacyKeyboard *keyboard);
static void listener_deregister(BackendLegacyKeyboard *keyboard);
static gboolean callback_keyboard(AtspiDeviceEvent *atspi_event, gpointer keyboard_ptr);

// create a new keyboard listener
BackendLegacyKeyboard *backend_legacy_keyboard_new(BackendLegacy *backend, BackendKeyboardCallback callback, gpointer data)
{
    BackendLegacyKeyboard *keyboard = g_new(BackendLegacyKeyboard, 1);

    // add backend
    keyboard->backend = backend;

    // add modifiers
    keyboard->modifiers = backend_legacy_modifiers_new(backend);

    // add callback
    keyboard->callback = callback;
    keyboard->data = data;

    // register listener
    keyboard->registered = FALSE;
    keyboard->listener = atspi_device_listener_new(callback_keyboard, keyboard, NULL);
    listener_register(keyboard);

    // initialize grabs
    keyboard->grabs = 0;
    keyboard->key_grabs = NULL;

    return keyboard;
}

// destroy the keyboard listener
void backend_legacy_keyboard_destroy(BackendLegacyKeyboard *keyboard)
{
    // deregister listener
    listener_deregister(keyboard);
    g_object_unref(keyboard->listener);

    // free grabs
    g_list_free_full(keyboard->key_grabs, g_free);

    // destroy modifiers
    backend_legacy_modifiers_destroy(keyboard->modifiers);

    // free
    g_free(keyboard);
}

// grab all keyboard input
void backend_legacy_keyboard_grab(BackendLegacyKeyboard *keyboard)
{
    // add a grab
    keyboard->grabs++;
}

// ungrab all keyboard input
void backend_legacy_keyboard_ungrab(BackendLegacyKeyboard *keyboard)
{
    // remove a grab
    if (keyboard->grabs > 0)
        keyboard->grabs--;
}

// grab input of a specific key
void backend_legacy_keyboard_grab_key(BackendLegacyKeyboard *keyboard, KeyboardEvent event)
{
    // sanitize modifier input
    event.modifiers = backend_legacy_modifiers_map(keyboard->modifiers, event.modifiers);

    // create grab
    KeyboardEvent *grab = g_new(KeyboardEvent, 1);
    *grab = event;

    // add the grab
    keyboard->key_grabs = g_list_append(keyboard->key_grabs, grab);
}

// ungrab input of a specific key
void backend_legacy_keyboard_ungrab_key(BackendLegacyKeyboard *keyboard, KeyboardEvent event)
{
    // sanitize modifier input
    event.modifiers = backend_legacy_modifiers_map(keyboard->modifiers, event.modifiers);

    // remove the first instance of the grab
    for (GList *link = keyboard->key_grabs; link; link = link->next)
    {
        KeyboardEvent *grab = link->data;

        // check if grab matches
        if (!((grab->keysym == event.keysym) &&
              (grab->type & event.type) &&
              (grab->modifiers == event.modifiers)))
            continue;

        // remove the grab
        g_free(grab);
        keyboard->key_grabs = g_list_delete_link(keyboard->key_grabs, link);
        return;
    }
}

static void listener_register(BackendLegacyKeyboard *keyboard)
{
    // do nothing if registered
    if (keyboard->registered)
        return;
    keyboard->registered = TRUE;

    // register to all keyboard events
    for (gint modifiers = 0; modifiers < 0xFF; modifiers++)
    {
        atspi_register_keystroke_listener(keyboard->listener,
                                          NULL,
                                          modifiers,
                                          KEYBOARD_EVENT_PRESSED | KEYBOARD_EVENT_RELEASED,
                                          ATSPI_KEYLISTENER_SYNCHRONOUS | ATSPI_KEYLISTENER_CANCONSUME,
                                          NULL);
    }
}

static void listener_deregister(BackendLegacyKeyboard *keyboard)
{
    // do nothing if not registered
    if (!keyboard->registered)
        return;
    keyboard->registered = FALSE;

    // deregister from all keyboard events
    for (gint modifiers = 0; modifiers < 0xFF; modifiers++)
    {
        atspi_deregister_keystroke_listener(keyboard->listener,
                                            NULL,
                                            modifiers,
                                            KEYBOARD_EVENT_PRESSED | KEYBOARD_EVENT_RELEASED,
                                            NULL);
    }
}

static gboolean callback_keyboard(AtspiDeviceEvent *atspi_event, gpointer keyboard_ptr)
{
    BackendLegacyKeyboard *keyboard = keyboard_ptr;

    // send the event
    KeyboardEvent event = {
        .keysym = atspi_event->id,
        .type = (atspi_event->type == ATSPI_KEY_PRESSED_EVENT) ? KEYBOARD_EVENT_PRESSED
                                                               : KEYBOARD_EVENT_RELEASED,
        .modifiers = backend_legacy_modifiers_map(keyboard->modifiers, atspi_event->modifiers),
    };
    keyboard->callback(event, keyboard->data);

    // free the event
    g_boxed_free(ATSPI_TYPE_DEVICE_EVENT, atspi_event);

    // check for global grab
    if (keyboard->grabs > 0)
        return KEYBOARD_EVENT_CONSUME;

    // check for a key grab
    for (GList *link = keyboard->key_grabs; link; link = link->next)
    {
        KeyboardEvent *grab = link->data;

        // check if grab matches
        if ((grab->keysym == event.keysym) &&
            (grab->type & event.type) &&
            (grab->modifiers == event.modifiers))
            return KEYBOARD_EVENT_CONSUME;
    }

    return KEYBOARD_EVENT_RELAY;
}
