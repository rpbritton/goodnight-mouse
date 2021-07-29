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

#include "../lib/keyboard/keyboard.h"
#include "../lib/keyboard/modifiers.h"
#include "../lib/mouse/mouse.h"

#include "execution.h"

static void callback_accessible_add(AtspiAccessible *accessible, gpointer foreground_ptr);
static void callback_accessible_remove(AtspiAccessible *accessible, gpointer foreground_ptr);

static KeyboardResponse callback_keyboard(KeyboardEvent event, gpointer foreground_ptr);
static MouseResponse callback_mouse(MouseEvent event, gpointer foreground_ptr);
static void callback_focus(AtspiAccessible *window, gpointer foreground_ptr);

// creates a new foreground that can be run
Foreground *foreground_new(ForegroundConfig *config, Focus *focus)
{
    Foreground *foreground = g_new(Foreground, 1);

    // create main loop
    foreground->loop = g_main_loop_new(NULL, FALSE);

    // create tag management
    foreground->accessible_to_tag = g_hash_table_new_full(NULL, NULL, g_object_unref, NULL);

    // add members
    foreground->focus = focus;

    // create members
    foreground->codes = codes_new(config->codes);
    foreground->overlay = overlay_new(config->overlay);
    foreground->registry = registry_new();

    return foreground;
}

// destroys and frees a foreground
void foreground_destroy(Foreground *foreground)
{
    // free members
    codes_destroy(foreground->codes);
    overlay_destroy(foreground->overlay);
    registry_destroy(foreground->registry);

    // free tag management
    g_hash_table_unref(foreground->accessible_to_tag);

    // free main loop
    g_main_loop_unref(foreground->loop);

    g_free(foreground);
}

// runs a foreground by starting a g main loop. stopped by calling quit.
void foreground_run(Foreground *foreground)
{
    if (foreground_is_running(foreground))
        return;

    // init shift state
    foreground->shifted = !!(keyboard_modifiers() & (GDK_SHIFT_MASK | GDK_LOCK_MASK));
    overlay_shifted(foreground->overlay, foreground->shifted);

    // get active window
    AtspiAccessible *window = focus_get_window(foreground->focus);
    if (!window)
    {
        g_warning("foreground: No active window, stopping.");
        return;
    }

    // let the registry watch the window
    registry_watch(foreground->registry, window, (RegistrySubscriber){
                                                     .add = callback_accessible_add,
                                                     .remove = callback_accessible_remove,
                                                     .data = foreground,
                                                 });

    // show the overlay
    overlay_show(foreground->overlay, window);
    // flush the main loop to have the window shown before listeners get registered
    while (g_main_context_iteration(NULL, FALSE))
        continue;

    // subscribe events
    focus_subscribe(foreground->focus, callback_focus, foreground);

    // create listeners
    KeyboardListener *keyboard_listener = keyboard_listener_new(callback_keyboard, foreground);
    MouseListener *mouse_listener = mouse_listener_new(callback_mouse, foreground);

    // run loop
    g_debug("foreground: Starting loop");
    g_main_loop_run(foreground->loop);
    g_debug("foreground: Stopping loop");

    // unsubscribe events
    focus_unsubscribe(foreground->focus, callback_focus);

    // destroy listeners
    keyboard_listener_destroy(keyboard_listener);
    mouse_listener_destroy(mouse_listener);

    // execute control
    Tag *tag = codes_matched_tag(foreground->codes);
    if (tag)
    {
        g_debug("foreground: Tag matched, executing control");
        execute_control(tag->accessible, foreground->shifted);
    }

    // clean up members
    registry_unwatch(foreground->registry);
    overlay_hide(foreground->overlay);
    g_object_unref(window);
}

// returns whether the given foreground is running
gboolean foreground_is_running(Foreground *foreground)
{
    return g_main_loop_is_running(foreground->loop);
}

// quits a given foreground if running
void foreground_quit(Foreground *foreground)
{
    if (!foreground_is_running(foreground))
        return;

    g_main_loop_quit(foreground->loop);
}

// event callback to a new accessible added
static void callback_accessible_add(AtspiAccessible *accessible, gpointer foreground_ptr)
{
    Foreground *foreground = foreground_ptr;

    // create tag
    Tag *tag = codes_allocate(foreground->codes);

    // set the accessible
    tag_set_accessible(tag, accessible);

    // add to the overlay
    overlay_add(foreground->overlay, tag);

    // add tag record
    g_hash_table_insert(foreground->accessible_to_tag, g_object_ref(accessible), tag);
}

// event callback to a previously added accessible being removed
static void callback_accessible_remove(AtspiAccessible *accessible, gpointer foreground_ptr)
{
    Foreground *foreground = foreground_ptr;

    // get the tag
    Tag *tag = g_hash_table_lookup(foreground->accessible_to_tag, accessible);

    // unset the accessible
    tag_unset_accessible(tag);

    // remove tag from overlay
    overlay_remove(foreground->overlay, tag);

    // deallocate tag
    codes_deallocate(foreground->codes, tag);

    // remove tag record
    g_hash_table_remove(foreground->accessible_to_tag, accessible);
}

// event callback for all keyboard events
static KeyboardResponse callback_keyboard(KeyboardEvent event, gpointer foreground_ptr)
{
    Foreground *foreground = foreground_ptr;

    // get shifted, note modifiers are set after the event
    gboolean shifted = !!(keyboard_modifiers() & (GDK_SHIFT_MASK | GDK_LOCK_MASK));
    // check if shift state changed
    if (shifted != foreground->shifted)
    {
        foreground->shifted = shifted;
        overlay_shifted(foreground->overlay, foreground->shifted);
    }

    // only check presses
    if (event.type != KEYBOARD_EVENT_PRESSED)
        return KEYBOARD_EVENT_CONSUME;
    g_debug("foreground: Received key press event for '%s'", gdk_keyval_name(event.key));

    // process key type
    switch (event.key)
    {
    case GDK_KEY_Escape:
        // quit on escape
        foreground_quit(foreground);
        break;
    case GDK_KEY_Up:
    case GDK_KEY_Down:
    case GDK_KEY_Left:
    case GDK_KEY_Right:
    case GDK_KEY_Page_Up:
    case GDK_KEY_Page_Down:
    case GDK_KEY_Home:
    case GDK_KEY_End:
        // pass these keys through to the window below
        return KEYBOARD_EVENT_RELAY;
        break;
    case GDK_KEY_BackSpace:
        // remove the last key
        codes_pop_key(foreground->codes);
        break;
    default:
        // add this pressed key
        codes_add_key(foreground->codes, event.key);
        // quit if matched
        if (codes_matched_tag(foreground->codes))
            foreground_quit(foreground);
        break;
    }

    return KEYBOARD_EVENT_CONSUME;
}

// event callback for all mouse events
static MouseResponse callback_mouse(MouseEvent event, gpointer foreground_ptr)
{
    g_debug("foreground: Received mouse button event");

    // mouse button, quit
    foreground_quit(foreground_ptr);
    return MOUSE_EVENT_RELAY;
}

// event callback for window focus changes
static void callback_focus(AtspiAccessible *window, gpointer foreground_ptr)
{
    g_debug("foreground: Received window focus change event");

    // window focus changed, quit
    foreground_quit(foreground_ptr);
}
