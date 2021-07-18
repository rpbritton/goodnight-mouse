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

Background *background_new(BackgroundConfig *config, Input *input, Foreground *foreground)
{
    Background *background = g_new(Background, 1);

    background->input = input;
    background->foreground = foreground;

    // add trigger
    // todo: include shift key for shifted state?
    background->trigger_event.type = INPUT_KEY_PRESSED | INPUT_KEY_RELEASED;
    background->trigger_event.id = config->trigger_id;
    background->trigger_event.modifiers = config->trigger_modifiers;

    // create main loop
    background->loop = g_main_loop_new(NULL, FALSE);

    return background;
}

void background_destroy(Background *background)
{
    // free main loop
    g_main_loop_unref(background->loop);

    g_free(background);
}

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

gboolean background_is_running(Background *background)
{
    return g_main_loop_is_running(background->loop);
}

void background_quit(Background *background)
{
    if (!background_is_running(background))
        return;

    foreground_quit(background->foreground);

    g_main_loop_quit(background->loop);
}

static InputResponse input_callback(InputEvent event, gpointer background_ptr)
{
    if (event.type == INPUT_KEY_PRESSED)
    {
        g_debug("background: Input hotkey triggered");
        g_idle_add_full(G_PRIORITY_HIGH, start_foreground, background_ptr, NULL);
    }

    return INPUT_CONSUME_EVENT;
}

static gboolean start_foreground(gpointer background_ptr)
{
    Background *background = (Background *)background_ptr;

    // run foreground
    foreground_run(background->foreground);

    return G_SOURCE_REMOVE;
}