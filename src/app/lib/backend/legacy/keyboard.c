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

#include <gdk/gdk.h>

#define KEY_EVENTS ((1 << ATSPI_KEY_PRESSED_EVENT) | (1 << ATSPI_KEY_PRESSED_EVENT))

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
void backend_legacy_keyboard_grab_key(BackendLegacyKeyboard *keyboard, BackendKeyboardEvent event)
{
    // add the grab
    BackendKeyboardEvent *grab = g_new(BackendKeyboardEvent, 1);
    *grab = event;
    keyboard->key_grabs = g_list_append(keyboard->key_grabs, grab);
}

// ungrab input of a specific key
void backend_legacy_keyboard_ungrab_key(BackendLegacyKeyboard *keyboard, BackendKeyboardEvent event)
{
    // remove the first instance of the grab
    for (GList *link = keyboard->key_grabs; link; link = link->next)
    {
        BackendKeyboardEvent *grab = link->data;

        // check if grab matches
        if (!((grab->keycode == event.keycode) &&
              (grab->state.modifiers == event.state.modifiers) &&
              (grab->state.group == event.state.group)))
            continue;

        // remove the grab
        g_free(grab);
        keyboard->key_grabs = g_list_delete_link(keyboard->key_grabs, link);
        return;
    }
}

BackendKeyboardState backend_legacy_keyboard_get_modifiers(BackendLegacyKeyboard *keyboard)
{
    // get state
    BackendKeyboardState state;
    state.modifiers = gdk_keymap_get_modifier_state(gdk_keymap_get_for_display(gdk_display_get_default())) & 0xFF;
    state.group = 0; // todo, maybe contained in the gdk return bits 13 and 14?

    // return
    return state;
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
                                          KEY_EVENTS,
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
                                            KEY_EVENTS,
                                            NULL);
    }
}

static gboolean callback_keyboard(AtspiDeviceEvent *atspi_event, gpointer keyboard_ptr)
{
    BackendLegacyKeyboard *keyboard = keyboard_ptr;

    // get event
    BackendKeyboardEvent event;
    event.keycode = atspi_event->hw_code;
    event.pressed = (atspi_event->type == ATSPI_KEY_PRESSED_EVENT);
    event.state.modifiers = atspi_event->modifiers & 0xFF;
    event.state.group = 0; // todo, maybe contained in the gdk return bits 13 and 14?

    // free the atspi event
    g_boxed_free(ATSPI_TYPE_DEVICE_EVENT, atspi_event);

    // send the event
    keyboard->callback(event, keyboard->data);

    // check for global grab
    if (keyboard->grabs > 0)
        return KEYBOARD_EVENT_CONSUME;

    // check for a key grab
    for (GList *link = keyboard->key_grabs; link; link = link->next)
    {
        BackendKeyboardEvent *grab = link->data;

        // check if a grab matches
        if ((grab->keycode == event.keycode) &&
            (grab->state.modifiers == event.state.modifiers) &&
            (grab->state.group == event.state.group))
            return KEYBOARD_EVENT_CONSUME;
    }

    return KEYBOARD_EVENT_RELAY;
}
