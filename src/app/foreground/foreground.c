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

#include "execution.h"

static void callback_accessible_add(AtspiAccessible *accessible, gpointer foreground_ptr);
static void callback_accessible_remove(AtspiAccessible *accessible, gpointer foreground_ptr);

static InputResponse callback_keyboard(InputEvent event, gpointer foreground_ptr);
static InputResponse callback_mouse(InputEvent event, gpointer foreground_ptr);
static void callback_focus(AtspiAccessible *window, gpointer foreground_ptr);

// input event that contains all key events
static const InputEvent KEYBOARD_EVENTS = {
    .type = INPUT_KEY_PRESSED | INPUT_KEY_RELEASED,
    .id = INPUT_ALL_IDS,
    .modifiers = INPUT_ALL_MODIFIERS,
};

// input event that contains all mouse events
static const InputEvent MOUSE_EVENTS = {
    .type = INPUT_BUTTON_PRESSED | INPUT_BUTTON_RELEASED,
    .id = INPUT_ALL_IDS,
    .modifiers = INPUT_ALL_MODIFIERS,
};

// creates a new foreground that can be run
Foreground *foreground_new(ForegroundConfig *config, Input *input, Focus *focus)
{
    Foreground *foreground = g_new(Foreground, 1);

    // create main loop
    foreground->loop = g_main_loop_new(NULL, FALSE);

    // create tag management
    foreground->accessible_to_tag = g_hash_table_new_full(NULL, NULL, g_object_unref, NULL);

    // add members
    foreground->input = input;
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
    foreground->shifted = !!(input_modifiers(foreground->input) & (GDK_SHIFT_MASK | GDK_LOCK_MASK));
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

    // execute control
    Tag *tag = codes_matched_tag(foreground->codes);
    if (tag)
        execute_control(foreground->input, tag->accessible, foreground->shifted);

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
static InputResponse callback_keyboard(InputEvent event, gpointer foreground_ptr)
{
    Foreground *foreground = foreground_ptr;

    // get shifted, note modifiers are set after the event
    gboolean shifted = !!(input_modifiers(foreground->input) & (GDK_SHIFT_MASK | GDK_LOCK_MASK));
    // check if shift state changed
    if (shifted != foreground->shifted)
    {
        foreground->shifted = shifted;
        overlay_shifted(foreground->overlay, foreground->shifted);
    }

    // only check presses
    if (event.type != INPUT_KEY_PRESSED)
        return INPUT_CONSUME_EVENT;

    // process key type
    switch (event.id)
    {
    case GDK_KEY_Escape:
        foreground_quit(foreground);
        break;
    case GDK_KEY_Up:
    case GDK_KEY_Down:
    case GDK_KEY_Left:
    case GDK_KEY_Right:
    case GDK_KEY_Page_Up:
    case GDK_KEY_Page_Down:
        return INPUT_RELAY_EVENT;
        break;
    case GDK_KEY_BackSpace:
        codes_pop_key(foreground->codes);
        break;
    default:
        codes_add_key(foreground->codes, event.id);
        // quit if matched
        if (codes_matched_tag(foreground->codes))
            foreground_quit(foreground);
        break;

        // todo: consider passing some keys through, like arrow keys
    }

    return INPUT_CONSUME_EVENT;
}

// event callback for all mouse events
static InputResponse callback_mouse(InputEvent event, gpointer foreground_ptr)
{
    foreground_quit(foreground_ptr);
    return INPUT_RELAY_EVENT;
}

// event callback for window focus changes
static void callback_focus(AtspiAccessible *window, gpointer foreground_ptr)
{
    if (window)
        g_object_unref(window);

    // window focus changed, quit
    foreground_quit(foreground_ptr);
}