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

#if USE_X11

#include "keyboard.h"

#include <X11/Xutil.h>

static void callback_xinput(XIDeviceEvent *xinput_event, gpointer keyboard_ptr);

// create a new keyboard listener
BackendX11Keyboard *backend_x11_keyboard_new(BackendX11 *backend, BackendKeyboardCallback callback, gpointer data)
{
    BackendX11Keyboard *keyboard = g_new(BackendX11Keyboard, 1);

    // add backend
    keyboard->backend = backend;

    // add callback
    keyboard->callback = callback;
    keyboard->data = data;

    // add x connection
    keyboard->display = backend_x11_get_display(keyboard->backend);
    keyboard->root_window = XDefaultRootWindow(keyboard->display);

    // add xinput
    keyboard->xinput = backend_x11_xinput_new(backend, callback_xinput, keyboard);

    return keyboard;
}

// destroy the keyboard listener
void backend_x11_keyboard_destroy(BackendX11Keyboard *keyboard)
{
    // destroy xinput
    backend_x11_xinput_destroy(keyboard->xinput);

    // free
    g_free(keyboard);
}

// grab all keyboard input
void backend_x11_keyboard_grab(BackendX11Keyboard *keyboard)
{
}

// ungrab all keyboard input
void backend_x11_keyboard_ungrab(BackendX11Keyboard *keyboard)
{
}

// grab input of a specific key
void backend_x11_keyboard_grab_key(BackendX11Keyboard *keyboard, KeyboardEvent event)
{
}

// ungrab input of a specific key
void backend_x11_keyboard_ungrab_key(BackendX11Keyboard *keyboard, KeyboardEvent event)
{
}

Modifiers backend_x11_keyboard_get_modifiers(BackendX11Keyboard *keyboard)
{
    // get pointer device
    int device_id;
    XIGetClientPointer(keyboard->display, None, &device_id);

    // get modifier state
    Window root, child;
    double root_x, root_y, win_x, win_y;
    XIButtonState buttons;
    XIModifierState mods;
    XIGroupState group;
    XIQueryPointer(keyboard->display, device_id, keyboard->root_window,
                   &root, &child, &root_x, &root_y, &win_x, &win_y, &buttons, &mods, &group);

    return mods.effective;
}

static void callback_xinput(XIDeviceEvent *xinput_event, gpointer keyboard_ptr)
{
    BackendX11Keyboard *keyboard = keyboard_ptr;

    // ensure this is a keyboard event
    if (xinput_event->evtype != XI_KeyPress &&
        xinput_event->evtype != XI_KeyRelease)
        return;

    // get event data
    KeyboardEvent event;

    // get keysym
    XKeyEvent x11_key_event;
    x11_key_event.display = xinput_event->display;
    x11_key_event.keycode = xinput_event->detail;
    x11_key_event.state = xinput_event->mods.effective;
    KeySym keysym;
    XLookupString(&x11_key_event, NULL, 0, &keysym, NULL);
    event.keysym = keysym;

    // get type
    event.type = (xinput_event->evtype == XI_KeyPress) ? KEYBOARD_EVENT_PRESSED : KEYBOARD_EVENT_RELEASED;

    // get modifiers
    event.modifiers = xinput_event->mods.effective;

    // callback the event
    keyboard->callback(event, keyboard->data);
}

#endif /* USE_X11 */
