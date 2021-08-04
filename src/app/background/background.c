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

static KeyboardResponse callback_key(KeyboardEvent event, gpointer background_ptr);
static gboolean start_foreground(gpointer background_ptr);
static void callback_focus(AtspiAccessible *window, gpointer background_ptr);

// creates a background that can be run
Background *background_new(BackgroundConfig *config, Foreground *foreground, FocusListener *focus_listener)
{
    Background *background = g_new(Background, 1);

    // create main loop
    background->loop = g_main_loop_new(NULL, FALSE);

    // add members
    background->focus_listener = focus_listener;
    background->foreground = foreground;

    // add trigger event
    background->trigger_event.type = KEYBOARD_EVENT_PRESSED | KEYBOARD_EVENT_RELEASED;
    background->trigger_event.key = config->key;
    background->trigger_event.modifiers = config->modifiers;

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
    // do nothing if running
    if (background_is_running(background))
        return;

    // subscribe to listeners
    KeyListener *trigger_listener = key_listener_new(background->trigger_event, callback_key, background);
    focus_listener_subscribe(background->focus_listener, callback_focus, background);

    // run loop
    g_debug("background: Starting loop");
    g_main_loop_run(background->loop);
    g_debug("background: Stopping loop");

    // unsubscribe from listeners
    key_listener_destroy(trigger_listener);
    focus_listener_unsubscribe(background->focus_listener, callback_focus);
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
static KeyboardResponse callback_key(KeyboardEvent event, gpointer background_ptr)
{
    if (event.type == KEYBOARD_EVENT_PRESSED)
    {
        g_debug("background: Input hotkey triggered");
        g_idle_add_full(G_PRIORITY_HIGH, start_foreground, background_ptr, NULL);
    }

    return KEYBOARD_EVENT_CONSUME;
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

// listens for focus events, which can help cache windows and improve speeds
static void callback_focus(AtspiAccessible *window, gpointer background_ptr)
{
    if (window)
    {
        const gchar *window_name = atspi_accessible_get_name(window, NULL);
        g_debug("background: Activated window '%s'", window_name);
        g_free((gpointer)window_name);
    }
    else
    {
        g_debug("background: Deactivated window");
    }
}
