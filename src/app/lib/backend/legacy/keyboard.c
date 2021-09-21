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
    keyboard->listener = atspi_device_listener_new(callback_keyboard, keyboard, NULL);
    for (gint modifiers = 0; modifiers < 0xFF; modifiers++)
    {
        atspi_register_keystroke_listener(keyboard->listener,
                                          NULL,
                                          modifiers,
                                          (1 << ATSPI_KEY_PRESSED_EVENT) | (1 << ATSPI_KEY_PRESSED_EVENT),
                                          ATSPI_KEYLISTENER_SYNCHRONOUS | ATSPI_KEYLISTENER_CANCONSUME,
                                          NULL);
    }

    return keyboard;
}

// destroy the keyboard listener
void backend_legacy_keyboard_destroy(BackendLegacyKeyboard *keyboard)
{
    // deregister listener
    for (gint modifiers = 0; modifiers < 0xFF; modifiers++)
    {
        atspi_deregister_keystroke_listener(keyboard->listener,
                                            NULL,
                                            modifiers,
                                            (1 << ATSPI_KEY_PRESSED_EVENT) | (1 << ATSPI_KEY_PRESSED_EVENT),
                                            NULL);
    }
    g_object_unref(keyboard->listener);

    // free
    g_free(keyboard);
}

// grab all keyboard input
void backend_legacy_keyboard_grab(BackendLegacyKeyboard *keyboard)
{
    // do nothing, keyboard is already grabbed by listener
}

// ungrab all keyboard input
void backend_legacy_keyboard_ungrab(BackendLegacyKeyboard *keyboard)
{
    // do nothing, keyboard is already grabbed by listener
}

// grab input of a specific key
void backend_legacy_keyboard_grab_key(BackendLegacyKeyboard *keyboard, guint keycode, BackendStateEvent state)
{
    // do nothing, keyboard is already grabbed by listener
}

// ungrab input of a specific key
void backend_legacy_keyboard_ungrab_key(BackendLegacyKeyboard *keyboard, guint keycode, BackendStateEvent state)
{
    // do nothing, keyboard is already grabbed by listener
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
    BackendKeyboardEventResponse response = keyboard->callback(event, keyboard->data);

    // tell atspi whether to consume
    return (response == BACKEND_KEYBOARD_EVENT_CONSUME) ? TRUE : FALSE;
}
