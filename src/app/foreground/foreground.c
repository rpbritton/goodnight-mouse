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

#define SHIFTED_MASK (GDK_SHIFT_MASK | GDK_LOCK_MASK)

static void callback_accessible_add(AtspiAccessible *accessible, gpointer foreground_ptr);
static void callback_accessible_remove(AtspiAccessible *accessible, gpointer foreground_ptr);

static KeyboardEventResponse callback_keyboard(KeyboardEvent event, gpointer foreground_ptr);
static PointerEventResponse callback_pointer(PointerEvent event, gpointer foreground_ptr);
static void callback_focus(AtspiAccessible *window, gpointer foreground_ptr);

// creates a new foreground that can be run
Foreground *foreground_new(ForegroundConfig *config, State *state, Emulator *emulator,
                           Keyboard *keyboard, Pointer *pointer, Focus *focus)
{
    Foreground *foreground = g_new(Foreground, 1);

    // create main loop
    foreground->loop = g_main_loop_new(NULL, FALSE);
    foreground->is_running = FALSE;

    // create tag management
    foreground->accessible_to_tag = g_hash_table_new_full(NULL, NULL, g_object_unref, NULL);

    // add dependencies
    foreground->state = state;
    foreground->emulator = emulator;
    foreground->keyboard = keyboard;
    foreground->pointer = pointer;
    foreground->focus = focus;

    // create members
    foreground->codes = codes_new(config->codes);
    foreground->overlay = overlay_new(config->overlay);
    foreground->registry = registry_new();
    foreground->executor = executor_new(emulator);

    return foreground;
}

// destroys and frees a foreground
void foreground_destroy(Foreground *foreground)
{
    // free members
    codes_destroy(foreground->codes);
    overlay_destroy(foreground->overlay);
    registry_destroy(foreground->registry);
    executor_destroy(foreground->executor);

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
    {
        g_debug("foreground: Foreground is already running");
        return;
    }

    // init shift state
    foreground->shifted = !!(state_get_modifiers(foreground->state) & SHIFTED_MASK);
    overlay_shifted(foreground->overlay, foreground->shifted);

    // get active window
    AtspiAccessible *window = focus_get_window(foreground->focus);
    if (!window)
    {
        g_warning("foreground: No active window, stopping");
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

    // subscribe to listeners
    keyboard_subscribe(foreground->keyboard, callback_keyboard, foreground);
    pointer_subscribe(foreground->pointer, callback_pointer, foreground);
    focus_subscribe(foreground->focus, callback_focus, foreground);

    // run loop
    g_debug("foreground: Starting loop");
    foreground->is_running = TRUE;
    g_main_loop_run(foreground->loop);
    foreground->is_running = FALSE;
    g_debug("foreground: Stopping loop");

    // unsubscribe from listeners
    keyboard_unsubscribe(foreground->keyboard, callback_keyboard, foreground);
    pointer_unsubscribe(foreground->pointer, callback_pointer, foreground);
    focus_unsubscribe(foreground->focus, callback_focus, foreground);

    // execute control
    Tag *tag = codes_matched_tag(foreground->codes);
    if (tag)
    {
        g_debug("foreground: Tag matched, executing control");
        executor_do(foreground->executor, tag->accessible, foreground->shifted);
    }

    // clean up members
    registry_unwatch(foreground->registry);
    overlay_hide(foreground->overlay);
    g_object_unref(window);
}

// returns whether the given foreground is running
gboolean foreground_is_running(Foreground *foreground)
{
    return foreground->is_running;
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
static KeyboardEventResponse callback_keyboard(KeyboardEvent event, gpointer foreground_ptr)
{
    Foreground *foreground = foreground_ptr;

    // modifiers are not set in the modifier key event, so get fresh set
    guint current_mods = state_get_modifiers(foreground->state);

    // check if shift state changed
    if (!!(current_mods & SHIFTED_MASK) != foreground->shifted)
    {
        foreground->shifted = !foreground->shifted;
        overlay_shifted(foreground->overlay, foreground->shifted);
    }

    // don't use key if modifiers other than the shift mods are held
    if (current_mods & ~SHIFTED_MASK)
        return KEYBOARD_EVENT_CONSUME;

    // process key type
    g_debug("foreground: Checking key %s event for '%s'", (event.pressed) ? "pressed" : "released", gdk_keyval_name(event.keysym));
    switch (event.keysym)
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
        //g_debug("foreground: Passing keysym (%d) to application", event.keysym);
        //keyboard_emulate_key(foreground->keyboard, event.keysym, 0);
        return KEYBOARD_EVENT_RELAY; // todo: send emulation
    case GDK_KEY_BackSpace:
        // only check pressed
        if (!event.pressed)
            break;
        // remove the last key
        codes_pop_key(foreground->codes);
        break;
    default:
        // only check pressed
        if (!event.pressed)
            break;
        // add this key
        codes_add_key(foreground->codes, event.keysym);
        // quit if matched
        if (codes_matched_tag(foreground->codes))
            foreground_quit(foreground);
        break;
    }

    return KEYBOARD_EVENT_CONSUME;
}

// event callback for all pointer events
static PointerEventResponse callback_pointer(PointerEvent event, gpointer foreground_ptr)
{
    g_debug("foreground: Pointer event received, stopping");

    // pointer button, quit
    foreground_quit(foreground_ptr);
    return POINTER_EVENT_RELAY;
}

// event callback for window focus changes
static void callback_focus(AtspiAccessible *window, gpointer foreground_ptr)
{
    g_debug("foreground: Window focus changed, stopping");

    // window focus changed, quit
    foreground_quit(foreground_ptr);
}
