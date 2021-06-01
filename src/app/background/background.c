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

InputEventAction trigger_callback(InputEvent event, gpointer data);

Background *background_new(Input *input, Foreground *foreground)
{
    Background *background = g_malloc(sizeof(Background));

    background->input = input;
    background->foreground = foreground;

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

void background_configure(Background *background, BackgroundConfig config)
{
    background->trigger_event = config.trigger_event;

    return;
}

void background_run(Background *background)
{
    input_subscribe(background->input, background->trigger_event, trigger_callback, background);

    g_main_loop_run(background->loop);

    input_unsubscribe(background->input, trigger_callback);
}

gboolean background_is_running(Background *background)
{
    return g_main_loop_is_running(background->loop);
}

void background_quit(Background *background)
{
    foreground_quit(background->foreground);

    g_main_loop_quit(background->loop);
}

InputEventAction trigger_callback(InputEvent event, gpointer data)
{
    printf("triggered!\n");

    return INPUT_CONSUME_EVENT;
}