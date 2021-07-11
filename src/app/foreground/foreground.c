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

#include "foreground.h"

static void callback_control_add(Control *control, gpointer foreground_ptr);
static void callback_control_remove(Control *control, gpointer foreground_ptr);

static InputResponse callback_keyboard(InputEvent event, gpointer foreground_ptr);
static InputResponse callback_mouse(InputEvent event, gpointer foreground_ptr);
static void callback_focus(AtspiAccessible *window, gpointer foreground_ptr);

static const InputEvent KEYBOARD_EVENTS = {
    .type = INPUT_KEY_PRESSED | INPUT_KEY_RELEASED,
    .id = INPUT_ALL_IDS,
    .modifiers = INPUT_ALL_MODIFIERS,
};

static const InputEvent MOUSE_EVENTS = {
    .type = INPUT_BUTTON_PRESSED | INPUT_BUTTON_RELEASED,
    .id = INPUT_ALL_IDS,
    .modifiers = INPUT_ALL_MODIFIERS,
};

Foreground *foreground_new(ForegroundConfig *config, Input *input, Focus *focus)
{
    Foreground *foreground = g_new(Foreground, 1);

    // add members
    foreground->input = input;
    foreground->focus = focus;

    foreground->codes = codes_new(config->keys);
    foreground->overlay = overlay_new(&config->overlay);
    foreground->registry = registry_new(callback_control_add, callback_control_remove, foreground);

    // create main loop
    foreground->loop = g_main_loop_new(NULL, FALSE);

    return foreground;
}

void foreground_destroy(Foreground *foreground)
{
    // free members
    codes_destroy(foreground->codes);
    overlay_destroy(foreground->overlay);
    registry_destroy(foreground->registry);

    // free main loop
    g_main_loop_unref(foreground->loop);

    g_free(foreground);
}

void foreground_run(Foreground *foreground)
{
    if (foreground_is_running(foreground))
        return;

    // get active window
    AtspiAccessible *window = focus_get_window(foreground->focus);
    if (!window)
    {
        g_warning("foreground: No active window, stopping.");
        return;
    }

    // let the registry watch the window
    registry_watch(foreground->registry, window);

    // check if there are any controls
    if (registry_count(foreground->registry) == 0)
    {
        g_warning("foreground: No controls found on active window, stopping.");
        registry_unwatch(foreground->registry);
        g_object_unref(window);
        return;
    }

    // show the overlay
    overlay_show(foreground->overlay, window);

    // subscribe events
    input_subscribe(foreground->input, KEYBOARD_EVENTS, callback_keyboard, foreground);
    input_subscribe(foreground->input, MOUSE_EVENTS, callback_mouse, foreground);
    focus_subscribe(foreground->focus, callback_focus, foreground);

    // run loop
    g_debug("foreground: Starting loop");
    g_main_loop_run(foreground->loop);
    g_debug("foreground: Stopping loop");

    // unsubscribe events
    input_unsubscribe(foreground->input, callback_keyboard);
    input_unsubscribe(foreground->input, callback_mouse);
    focus_unsubscribe(foreground->focus, callback_focus);

    // clean up members
    registry_unwatch(foreground->registry);
    overlay_hide(foreground->overlay);
    g_object_unref(window);
}

gboolean foreground_is_running(Foreground *foreground)
{
    return g_main_loop_is_running(foreground->loop);
}

void foreground_quit(Foreground *foreground)
{
    if (!foreground_is_running(foreground))
        return;

    g_main_loop_quit(foreground->loop);
}

static void callback_control_add(Control *control, gpointer foreground_ptr)
{
    Foreground *foreground = foreground_ptr;

    // add the code
    codes_add(foreground->codes, control);

    // add to overlay
    overlay_add(foreground->overlay, control);

    //g_message("control added (code=%d)", g_array_index(control->code, guint, 0));
}

static void callback_control_remove(Control *control, gpointer foreground_ptr)
{
    Foreground *foreground = foreground_ptr;

    // remove the code
    codes_remove(foreground->codes, control);

    // remove from overlay
    overlay_remove(foreground->overlay, control);

    //g_message("control removed");
}

static InputResponse callback_keyboard(InputEvent event, gpointer foreground_ptr)
{
    if (event.type != INPUT_KEY_PRESSED)
        return INPUT_CONSUME_EVENT;

    switch (event.id)
    {
    case GDK_KEY_Escape:
        foreground_quit(foreground_ptr);
        break;
    default:
        // todo: check controls for matches
        break;
    }

    return INPUT_CONSUME_EVENT;
}

static InputResponse callback_mouse(InputEvent event, gpointer foreground_ptr)
{
    foreground_quit(foreground_ptr);
    return INPUT_RELAY_EVENT;
}

static void callback_focus(AtspiAccessible *window, gpointer foreground_ptr)
{
    if (window)
        g_object_unref(window);

    // window focus changed, quit
    foreground_quit(foreground_ptr);
}