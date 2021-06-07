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

static InputResponse input_callback(InputEvent event, gpointer foreground_ptr);

static InputEvent all_events = {
    .type = INPUT_ALL_TYPES,
    .id = INPUT_ALL_IDS,
    .modifiers = INPUT_ALL_MODIFIERS,
};

Foreground *foreground_new(Input *input)
{
    Foreground *foreground = g_malloc(sizeof(Foreground));

    foreground->input = input;

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

    input_subscribe(foreground->input, all_events, input_callback, foreground);

    g_main_loop_run(foreground->loop);

    input_unsubscribe(foreground->input, input_callback);
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

static InputResponse input_callback(InputEvent event, gpointer foreground_ptr)
{
    Foreground *foreground = (Foreground *)foreground_ptr;

    if (event.type == INPUT_KEY_PRESSED)
    {
        switch (event.id)
        {
        case GDK_KEY_Escape:
            foreground_quit(foreground);
            return INPUT_CONSUME_EVENT;
        }
    }
    else if (event.type & (INPUT_BUTTON_PRESSED | INPUT_BUTTON_RELEASED))
    {
        switch (event.id)
        {
        default:
            foreground_quit(foreground);
            return INPUT_RELAY_EVENT;
        }
    }

    return INPUT_CONSUME_EVENT;
}