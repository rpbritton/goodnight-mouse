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
#include <X11/extensions/XInput2.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XTest.h>

#include "utils.h"

static void emulate_start(BackendX11Keyboard *keyboard);
static void emulate_key(BackendX11Keyboard *keyboard, guint keycode, gboolean pressed);

static void set_grab(BackendX11Keyboard *keyboard);
static void unset_grab(BackendX11Keyboard *keyboard);

static void reset_key_grab(BackendX11Keyboard *keyboard, BackendKeyboardEvent *grab);
static void set_key_grab(BackendX11Keyboard *keyboard, BackendKeyboardEvent *grab);
static void unset_key_grab(BackendX11Keyboard *keyboard, BackendKeyboardEvent *grab);

static void callback_xinput(XEvent *x11_event, gpointer keyboard_ptr);
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

    // initialize grabs
    keyboard->grabs = 0;
    keyboard->key_grabs = NULL;

    // add x connection
    keyboard->display = backend_x11_get_display(keyboard->backend);
    keyboard->root_window = XDefaultRootWindow(keyboard->display);

    // subscribe to xinput
    backend_x11_subscribe(keyboard->backend, BACKEND_X11_EVENT_TYPE_XINPUT, XI_KeyPress, callback_xinput, keyboard);
    backend_x11_subscribe(keyboard->backend, BACKEND_X11_EVENT_TYPE_XINPUT, XI_KeyRelease, callback_xinput, keyboard);

    // get primary keyboard id
    keyboard->keyboard_id = get_device_id(keyboard->display, XIMasterKeyboard);

    // add focus listener
    keyboard->focus = backend_x11_focus_new(keyboard->backend, callback_focus, keyboard);
    keyboard->grab_window = backend_x11_focus_get_x11_window(keyboard->focus);

    // init last event
    keyboard->last_event.keycode = 0;

    // add emulation tracker
    keyboard->is_emulating = FALSE;
    keyboard->emulated_keys = g_hash_table_new(NULL, NULL);

    return keyboard;
}

// destroy the keyboard listener
void backend_x11_keyboard_destroy(BackendX11Keyboard *keyboard)
{
    // free emulation tracker
    backend_x11_keyboard_emulate_reset(keyboard);
    g_hash_table_unref(keyboard->emulated_keys);

    // unsubscribe from xinput
    backend_x11_unsubscribe(keyboard->backend, BACKEND_X11_EVENT_TYPE_XINPUT, XI_KeyPress, callback_xinput, keyboard);
    backend_x11_unsubscribe(keyboard->backend, BACKEND_X11_EVENT_TYPE_XINPUT, XI_KeyRelease, callback_xinput, keyboard);

    // free grabs
    g_list_free_full(keyboard->key_grabs, g_free);

    // free focus listener
    backend_x11_focus_destroy(keyboard->focus);

    // free
    g_free(keyboard);
}

// grab all keyboard input
void backend_x11_keyboard_grab(BackendX11Keyboard *keyboard)
{
    // add a grab
    keyboard->grabs++;

    // add grab if not set
    if (keyboard->grabs == 1)
        set_grab(keyboard);
}

// ungrab all keyboard input
void backend_x11_keyboard_ungrab(BackendX11Keyboard *keyboard)
{
    // check if the grab exists
    if (keyboard->grabs == 0)
        return;

    // remove a grab
    keyboard->grabs--;

    // remove the grab if none exist now
    if (keyboard->grabs == 0)
        unset_grab(keyboard);
}

// grab input of a specific key
void backend_x11_keyboard_grab_key(BackendX11Keyboard *keyboard, BackendKeyboardEvent event)
{
    // allocate grab
    BackendKeyboardEvent *grab = g_new(BackendKeyboardEvent, 1);
    *grab = event;

    // add grab
    keyboard->key_grabs = g_list_append(keyboard->key_grabs, grab);

    // reset grab
    reset_key_grab(keyboard, grab);
}

// ungrab input of a specific key
void backend_x11_keyboard_ungrab_key(BackendX11Keyboard *keyboard, BackendKeyboardEvent event)
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

        // remove grab
        keyboard->key_grabs = g_list_delete_link(keyboard->key_grabs, link);

        // reset grab
        reset_key_grab(keyboard, grab);

        // free grab
        g_free(grab);
        return;
    }
}

BackendKeyboardState backend_x11_keyboard_get_state(BackendX11Keyboard *keyboard)
{
    // get state
    XkbStateRec xkb_state;
    XkbGetState(keyboard->display, XkbUseCoreKbd, &xkb_state);

    // parse state
    BackendKeyboardState state;
    state.modifiers = xkb_state.mods & 0xFF;
    state.group = xkb_state.group & 0xFF;

    // return
    return state;
}

void backend_x11_keyboard_emulate_reset(BackendX11Keyboard *keyboard)
{
    // only reset if emulating
    if (!keyboard->is_emulating)
        return;

    // set the initial state
    backend_x11_keyboard_emulate_state(keyboard, keyboard->initial_state);

    // get lists of keys to send
    GList *keys_to_press = NULL;
    GList *keys_to_release = NULL;
    GHashTableIter iter;
    gpointer keycode, pressed;
    g_hash_table_iter_init(&iter, keyboard->emulated_keys);
    while (g_hash_table_iter_next(&iter, &keycode, &pressed))
        if (GPOINTER_TO_UINT(pressed))
            keys_to_release = g_list_prepend(keys_to_release, keycode);
        else
            keys_to_press = g_list_prepend(keys_to_press, keycode);

    // send keys
    for (GList *link = keys_to_press; link; link = link->next)
        emulate_key(keyboard, GPOINTER_TO_UINT(link->data), TRUE);
    g_list_free(keys_to_press);
    for (GList *link = keys_to_release; link; link = link->next)
        emulate_key(keyboard, GPOINTER_TO_UINT(link->data), FALSE);
    g_list_free(keys_to_release);

    // set not emulating
    keyboard->is_emulating = FALSE;
}

void backend_x11_keyboard_emulate_state(BackendX11Keyboard *keyboard, BackendKeyboardState state)
{
    // start emulating
    emulate_start(keyboard);

    // get current state
    BackendKeyboardState current_state = backend_x11_keyboard_get_state(keyboard);

    // get keycodes to set modifiers
    XModifierKeymap *modifiers = XGetModifierMapping(keyboard->display);

    // check all the modifiers
    for (gint mod_index = 0; mod_index <= 8; mod_index++)
    {
        // do nothing if already correct
        if ((state.modifiers & (1 << mod_index)) == (current_state.modifiers & (1 << mod_index)))
            continue;

        // get modifier action
        gboolean press = (state.modifiers & (1 << mod_index)) && !(current_state.modifiers & (1 << mod_index));

        // try sending keycodes to set the modifier
        for (gint keycode_index = 0; keycode_index < modifiers->max_keypermod; keycode_index++)
        {
            // get the keycode
            guint keycode = modifiers->modifiermap[mod_index * modifiers->max_keypermod + keycode_index];
            if (!keycode)
                continue;

            // send key
            emulate_key(keyboard, keycode, press);

            // check if was set
            current_state = backend_x11_keyboard_get_state(keyboard);
            if (((state.modifiers & (1 << mod_index))) != (current_state.modifiers & (1 << mod_index)))
            {
                emulate_key(keyboard, keycode, !press);
                current_state = backend_x11_keyboard_get_state(keyboard);
            }
            else
            {
                break;
            }
        }
    }

    // free
    XFreeModifiermap(modifiers);

    // set the group
    XkbLockGroup(keyboard->display, XkbUseCoreKbd, state.group);
}

void backend_x11_keyboard_emulate_key(BackendX11Keyboard *keyboard, BackendKeyboardEvent event)
{
    // start emulating
    emulate_start(keyboard);

    // set the state
    backend_x11_keyboard_emulate_state(keyboard, event.state);

    // send the key event
    emulate_key(keyboard, event.keycode, event.pressed);
}

static void emulate_start(BackendX11Keyboard *keyboard)
{
    // don't start if already started
    if (keyboard->is_emulating)
        return;

    // set emulating
    keyboard->is_emulating = TRUE;
    keyboard->initial_state = backend_x11_keyboard_get_state(keyboard);
}

static void emulate_key(BackendX11Keyboard *keyboard, guint keycode, gboolean pressed)
{
    // send key
    XTestFakeKeyEvent(keyboard->display, keycode, pressed, CurrentTime);

    // sync the display
    XSync(keyboard->display, FALSE);

    // record in emulation table
    if (g_hash_table_contains(keyboard->emulated_keys, GUINT_TO_POINTER(keycode)))
    {
        if (GPOINTER_TO_UINT(g_hash_table_lookup(keyboard->emulated_keys, GUINT_TO_POINTER(keycode))) != pressed)
            g_hash_table_remove(keyboard->emulated_keys, GUINT_TO_POINTER(keycode));
    }
    else
    {
        g_hash_table_insert(keyboard->emulated_keys, GUINT_TO_POINTER(keycode), GUINT_TO_POINTER(pressed));
    }
}

static void set_grab(BackendX11Keyboard *keyboard)
{
    g_debug("x11-keyboard: Attempting global keyboard grab");

    // grab the device
    XIEventMask event_mask = {.deviceid = keyboard->keyboard_id, .mask_len = 0, .mask = NULL};
    gint status = XIGrabDevice(keyboard->display, keyboard->keyboard_id, keyboard->grab_window, CurrentTime,
                               None, XIGrabModeSync, XIGrabModeSync, FALSE, &event_mask);

    // check if successful
    if (status == AlreadyGrabbed)
        g_warning("x11-keyboard: Failed global keyboard grab");
}

static void unset_grab(BackendX11Keyboard *keyboard)
{
    g_debug("x11-keyboard: Ungrabbing keyboard");
    XIUngrabDevice(keyboard->display, keyboard->keyboard_id, CurrentTime);
}

static void reset_key_grab(BackendX11Keyboard *keyboard, BackendKeyboardEvent *grab)
{
    // check if grab exists
    for (GList *link = keyboard->key_grabs; link; link = link->next)
    {
        BackendKeyboardEvent *found_grab = link->data;

        // check if grab matches
        if (!((grab->keycode == found_grab->keycode) &&
              (grab->state.modifiers == found_grab->state.modifiers) &&
              (grab->state.group == found_grab->state.group)))
            continue;

        // set grab
        set_key_grab(keyboard, grab);
        return;
    }

    // unset grab
    unset_key_grab(keyboard, grab);
}

static void set_key_grab(BackendX11Keyboard *keyboard, BackendKeyboardEvent *grab)
{
    XIEventMask event_mask = {.deviceid = keyboard->keyboard_id, .mask_len = 0, .mask = NULL};
    XIGrabModifiers modifier_input;
    // todo, this doesn't take group into the account
    modifier_input.modifiers = grab->state.modifiers;
    int status = XIGrabKeycode(keyboard->display, keyboard->keyboard_id, grab->keycode, keyboard->grab_window,
                               XIGrabModeSync, XIGrabModeSync, FALSE, &event_mask, 1, &modifier_input);

    // check if successful
    if (status == AlreadyGrabbed)
        g_debug("x11-keyboard: Failed to grab key: keycode: %d, modifiers: 0x%X", grab->keycode, modifier_input.modifiers);
}

static void unset_key_grab(BackendX11Keyboard *keyboard, BackendKeyboardEvent *grab)
{
    XIGrabModifiers modifier_input;
    // todo, this doesn't take group into the account
    modifier_input.modifiers = grab->state.modifiers;
    XIUngrabKeycode(keyboard->display, keyboard->keyboard_id, grab->keycode, keyboard->grab_window,
                    1, &modifier_input);
}

static void callback_xinput(XEvent *x11_event, gpointer keyboard_ptr)
{
    BackendX11Keyboard *keyboard = keyboard_ptr;

    // get device event
    XIDeviceEvent *device_event = x11_event->xcookie.data;

    // get event data
    BackendKeyboardEvent event;
    event.keycode = device_event->detail;
    event.pressed = (device_event->evtype == XI_KeyPress);
    event.state.modifiers = device_event->mods.effective;
    event.state.group = device_event->group.effective;

    // check for duplicate event
    if (device_event->time == keyboard->last_event_time &&
        event.keycode == keyboard->last_event.keycode &&
        event.pressed == keyboard->last_event.pressed &&
        event.state.modifiers == keyboard->last_event.state.modifiers &&
        event.state.group == keyboard->last_event.state.group)
        return;
    keyboard->last_event_time = device_event->time;
    keyboard->last_event = event;

    // callback the event
    BackendKeyboardEventResponse response = keyboard->callback(event, keyboard->data);

    // consume or relay the event
    if (response == BACKEND_KEYBOARD_EVENT_CONSUME)
    {
        g_message("consuming");
        XIAllowEvents(keyboard->display, keyboard->keyboard_id,
                      XISyncDevice, device_event->time);
    }
    else
    {
        g_message("replaying");
        XIAllowEvents(keyboard->display, keyboard->keyboard_id,
                      XIReplayDevice, device_event->time);
        XFlush(keyboard->display);

        // unthaw the keyboard
        if (keyboard->grabs > 0)
        {
            unset_grab(keyboard);
            set_grab(keyboard);
        }
    }
}

static void callback_focus(gpointer keyboard_ptr)
{
    BackendX11Keyboard *keyboard = keyboard_ptr;

    // set the new active window to grab
    // this prevents the window from losing focus on key grabs
    keyboard->grab_window = backend_x11_focus_get_x11_window(keyboard->focus);

    // set the global grab to the new window
    if (keyboard->grabs > 0)
        set_grab(keyboard);

    // set the key grabs to the new window
    for (GList *link = keyboard->key_grabs; link; link = link->next)
        set_key_grab(keyboard, link->data);
}

#endif /* USE_X11 */
