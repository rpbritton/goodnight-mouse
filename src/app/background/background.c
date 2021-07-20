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

#include "background.h"

static InputResponse input_callback(InputEvent event, gpointer background_ptr);
static gboolean start_foreground(gpointer background_ptr);

// creates a background that can be run
Background *background_new(BackgroundConfig *config, Input *input, Foreground *foreground)
{
    Background *background = g_new(Background, 1);

    background->input = input;
    background->foreground = foreground;

    // add trigger
    background->trigger_event.type = INPUT_KEY_PRESSED | INPUT_KEY_RELEASED;
    background->trigger_event.id = config->key;
    background->trigger_event.modifiers = config->modifiers;

    // create main loop
    background->loop = g_main_loop_new(NULL, FALSE);

    return background;
}

// destroys a background
void background_destroy(Background *background)
{
    // free main loop
    g_main_loop_unref(background->loop);

    g_free(background);
}

// runs the background until quit is called
void background_run(Background *background)
{
    if (background_is_running(background))
        return;

    // subscribe events
    input_subscribe(background->input, background->trigger_event, input_callback, background);

    // run loop
    g_debug("background: Starting loop");
    g_main_loop_run(background->loop);
    g_debug("background: Stopping loop");

    // unsubscribe events
    input_unsubscribe(background->input, input_callback);
}

// returns whether the background is running
gboolean background_is_running(Background *background)
{
    return g_main_loop_is_running(background->loop);
}

// quits the background if running
void background_quit(Background *background)
{
    if (!background_is_running(background))
        return;

    foreground_quit(background->foreground);

    g_main_loop_quit(background->loop);
}

// callback to handle the hotkey input event by scheduling the foreground
// to start
static InputResponse input_callback(InputEvent event, gpointer background_ptr)
{
    if (event.type == INPUT_KEY_PRESSED)
    {
        g_debug("background: Input hotkey triggered");
        g_idle_add_full(G_PRIORITY_HIGH, start_foreground, background_ptr, NULL);
    }

    return INPUT_CONSUME_EVENT;
}

// starts the foreground from inside a source, which will be removed on foreground
// completion
static gboolean start_foreground(gpointer background_ptr)
{
    Background *background = (Background *)background_ptr;

    // run foreground
    foreground_run(background->foreground);

    return G_SOURCE_REMOVE;
}