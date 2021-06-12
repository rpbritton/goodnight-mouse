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

static InputResponse keyboard_callback(InputEvent event, gpointer foreground_ptr);
static InputResponse mouse_callback(InputEvent event, gpointer foreground_ptr);
static void focus_callback(AtspiAccessible *window, gpointer foreground_ptr);

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

{
    Foreground *foreground = g_new(Foreground, 1);

    foreground->input = input;
    foreground->focus = focus;
    foreground->actions = actions;

    // create main loop
    foreground->loop = g_main_loop_new(NULL, FALSE);

    return foreground;
}

void foreground_destroy(Foreground *foreground)
{
    // free main loop
    g_main_loop_unref(foreground->loop);

    g_free(foreground);
}

void foreground_configure(Foreground *foreground, ForegroundConfig config)
{
    return;
}

void foreground_run(Foreground *foreground)
{
    if (foreground_is_running(foreground))
        return;

    // subscribe events
    input_subscribe(foreground->input, KEYBOARD_EVENTS, keyboard_callback, foreground);
    input_subscribe(foreground->input, MOUSE_EVENTS, mouse_callback, foreground);
    focus_subscribe(foreground->focus, focus_callback, foreground);

    // run loop
    g_debug("foreground: Starting loop");
    g_main_loop_run(foreground->loop);
    g_debug("foreground: Stopping loop");

    // unsubscribe events
    input_unsubscribe(foreground->input, keyboard_callback);
    input_unsubscribe(foreground->input, mouse_callback);
    focus_unsubscribe(foreground->focus, focus_callback);
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

static InputResponse keyboard_callback(InputEvent event, gpointer foreground_ptr)
{
    if (event.type != INPUT_KEY_PRESSED)
        return INPUT_CONSUME_EVENT;

    switch (event.id)
    {
    case GDK_KEY_Escape:
        foreground_quit(foreground_ptr);
        break;
    default:
        // todo: notify controller
        break;
    }

    return INPUT_CONSUME_EVENT;
}

static InputResponse mouse_callback(InputEvent event, gpointer foreground_ptr)
{
    foreground_quit(foreground_ptr);
    return INPUT_RELAY_EVENT;
}

static void focus_callback(AtspiAccessible *window, gpointer foreground_ptr)
{
    // window focus changed, quit
    Foreground *foreground = (Foreground *)foreground_ptr;
    foreground_quit(foreground);

    if (window)
        g_object_unref(window);
}