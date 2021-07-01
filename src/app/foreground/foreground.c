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

Foreground *foreground_new(Input *input, Focus *focus)
{
    Foreground *foreground = g_new(Foreground, 1);

    // add members
    foreground->input = input;
    foreground->focus = focus;

    foreground->registry = registry_new(callback_control_add, callback_control_remove, foreground);

    // create main loop
    foreground->loop = g_main_loop_new(NULL, FALSE);

    return foreground;
}

void foreground_destroy(Foreground *foreground)
{
    // free members
    registry_destroy(foreground->registry);

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

    // get active window
    AtspiAccessible *window = focus_window(foreground->focus);
    if (!window)
    {
        g_warning("foreground: No active window, stopping.");
        return;
    }

    // let the registry watch the window
    registry_watch(foreground->registry, window);
    g_object_unref(window);

    // check if there are no controls
    if (registry_count(foreground->registry) == 0)
    {
        g_warning("foreground: No controls found on active window, stopping.");
        registry_reset(foreground->registry);
        return;
    }

    // subscribe events
    input_subscribe(foreground->input, KEYBOARD_EVENTS, callback_keyboard, foreground);
    input_subscribe(foreground->input, MOUSE_EVENTS, callback_mouse, foreground);
    focus_subscribe(foreground->focus, callback_focus, foreground);

    //g_message("fetching list");
    //struct timeval tval_before, tval_after, tval_result;
    //gettimeofday(&tval_before, NULL);
    //GList *list = registry_list(foreground->registry);
    //gettimeofday(&tval_after, NULL);
    //timersub(&tval_after, &tval_before, &tval_result);
    //gint length = g_list_length(list);
    //g_message("got list: length: %d, time: %ld.%06ld", length, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
    //
    //gettimeofday(&tval_before, NULL);
    //for (GList *control = list; control; control = control->next)
    //{
    //    AtspiAccessible *accessible = (AtspiAccessible *)control->data;
    //    AtspiRole role = atspi_accessible_get_role(accessible, NULL);
    //}
    //gettimeofday(&tval_after, NULL);
    //timersub(&tval_after, &tval_before, &tval_result);
    //g_message("time to get roles 1: %ld.%06ld", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
    //
    //gettimeofday(&tval_before, NULL);
    //for (GList *control = list; control; control = control->next)
    //{
    //    AtspiAccessible *accessible = (AtspiAccessible *)control->data;
    //    AtspiRole role = atspi_accessible_get_role(accessible, NULL);
    //}
    //gettimeofday(&tval_after, NULL);
    //timersub(&tval_after, &tval_before, &tval_result);
    //g_message("time to get roles 2: %ld.%06ld", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

    // run loop
    g_debug("foreground: Starting loop");
    g_main_loop_run(foreground->loop);
    g_debug("foreground: Stopping loop");

    // reset members
    registry_reset(foreground->registry);

    // unsubscribe events
    input_unsubscribe(foreground->input, callback_keyboard);
    input_unsubscribe(foreground->input, callback_mouse);
    focus_unsubscribe(foreground->focus, callback_focus);
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
    Foreground *foreground = (Foreground *)foreground_ptr;

    codes_add_control(foreground->codes, control);
    overlay_add_control(foreground->overlay, control);
}

static void callback_control_remove(Control *control, gpointer foreground_ptr)
{
    Foreground *foreground = (Foreground *)foreground_ptr;

    codes_remove_control(foreground->codes, control);
    overlay_remove_control(foreground->overlay, control);
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
        // todo: notify registry
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
    Foreground *foreground = (Foreground *)foreground_ptr;
    foreground_quit(foreground);
}