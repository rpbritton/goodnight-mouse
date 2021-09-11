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

#include "utils.h"

// typedef struct KeyGrab
// {
//     BackendKeyboardEvent event;
//     int keycode;
// } KeyGrab;

// static void set_grab(BackendX11Keyboard *keyboard);
static void callback_xinput(XIDeviceEvent *xinput_event, gpointer keyboard_ptr);
static void callback_focus(gpointer keyboard_ptr);

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

    // get device ids
    keyboard->keyboard_id = get_device_id(keyboard->display, XIMasterKeyboard);
    keyboard->pointer_id = get_device_id(keyboard->display, XIMasterPointer);

    // add xinput
    keyboard->xinput = backend_x11_xinput_new(backend, callback_xinput, keyboard);

    // initialize grabs
    keyboard->grabs = 0;
    keyboard->key_grabs = NULL;

    // add focus listener
    keyboard->focus = backend_x11_focus_new(keyboard->backend, callback_focus, keyboard);
    keyboard->grab_window = backend_x11_focus_get_x11_window(keyboard->focus);

    return keyboard;
}

// destroy the keyboard listener
void backend_x11_keyboard_destroy(BackendX11Keyboard *keyboard)
{
    // destroy xinput
    backend_x11_xinput_destroy(keyboard->xinput);

    // free grabs
    g_list_free_full(keyboard->key_grabs, g_free);

    // destroy the focus listener
    backend_x11_focus_destroy(keyboard->focus);

    // free
    g_free(keyboard);
}

// grab all keyboard input
void backend_x11_keyboard_grab(BackendX11Keyboard *keyboard)
{
    // // add a grab
    // keyboard->grabs++;

    // // add grab
    // set_grab(keyboard);
}

// ungrab all keyboard input
void backend_x11_keyboard_ungrab(BackendX11Keyboard *keyboard)
{
    // // check if the grab exists
    // if (keyboard->grabs == 0)
    //     return;

    // // remove a grab
    // keyboard->grabs--;

    // // remove the grab if none exist now
    // if (keyboard->grabs == 0)
    //     XIUngrabDevice(keyboard->display, keyboard->keyboard_id, CurrentTime);
}

// grab input of a specific key
void backend_x11_keyboard_grab_key(BackendX11Keyboard *keyboard, BackendKeyboardEvent event)
{
}

// ungrab input of a specific key
void backend_x11_keyboard_ungrab_key(BackendX11Keyboard *keyboard, BackendKeyboardEvent event)
{
}

BackendKeyboardState backend_x11_keyboard_get_modifiers(BackendX11Keyboard *keyboard)
{
    // get the current state
    Window root, child;
    double root_x, root_y, win_x, win_y;
    XIButtonState buttons;
    XIModifierState mods;
    XIGroupState group;
    XIQueryPointer(keyboard->display, keyboard->pointer_id, keyboard->root_window,
                   &root, &child, &root_x, &root_y, &win_x, &win_y, &buttons, &mods, &group);

    // parse the current state
    BackendKeyboardState state;
    state.modifiers = mods.effective & 0xFF;
    state.group = group.effective & 0xFF;

    // return
    return state;
}

// static void set_grabs(BackendX11Keyboard *keyboard)
// {
//     XIEventMask event_mask = {
//         .deviceid = XIAllDevices,
//         .mask_len = 0,
//         .mask = NULL,
//     };

//     // check if any grabs exist
//     //if (keyboard->grabs > 0)
//     if (FALSE)
//     {
//         XIGrabDevice(keyboard->display, keyboard->keyboard_id, keyboard->grab_window,
//                      CurrentTime,   // grabbing now
//                      None,          // use default cursor <-- todo verify
//                      GrabModeAsync, // todo
//                      GrabModeAsync, // todo
//                      False,         // todo
//                      &event_mask);  // todo
//     }

//     // set any key grabs
//     for (GList *link = keyboard->key_grabs; link; link = link->next)
//     {
//         KeyGrab *grab = link->data;
//         XIGrabKeycode(keyboard->display, keyboard->keyboard_id, grab->keycode,
//                       keyboard->grab_window,
//                       GrabModeAsync, // todo
//                       GrabModeAsync, // todo
//                       False,         // todo
//                       &event_mask,   // todo
//                       grab->event.modifiers,
//                       NULL); // todo
//     }
// }

static void callback_xinput(XIDeviceEvent *xinput_event, gpointer keyboard_ptr)
{
    BackendX11Keyboard *keyboard = keyboard_ptr;

    // ensure this is a keyboard event
    if (xinput_event->evtype != XI_KeyPress &&
        xinput_event->evtype != XI_KeyRelease)
        return;

    // get event data
    BackendKeyboardEvent event;
    event.keycode = xinput_event->detail;
    event.pressed = (xinput_event->evtype == XI_KeyPress);
    event.state.modifiers = xinput_event->mods.effective;
    event.state.group = xinput_event->group.effective;

    // callback the event
    keyboard->callback(event, keyboard->data);
}

static void callback_focus(gpointer keyboard_ptr)
{
    BackendX11Keyboard *keyboard = keyboard_ptr;

    // get new focused window
    keyboard->grab_window = backend_x11_focus_get_x11_window(keyboard->focus);

    // set the grabs
    // set_grabs(keyboard);
}

#endif /* USE_X11 */
