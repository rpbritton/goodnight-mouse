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

static void check_tag_matched(Foreground *foreground);

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

Foreground *foreground_new(ForegroundConfig *config, Input *input, Focus *focus)
{
    Foreground *foreground = g_new(Foreground, 1);

    // create main loop
    foreground->loop = g_main_loop_new(NULL, FALSE);

    // create control management
    foreground->controls_to_tags = g_hash_table_new(NULL, NULL);

    // add members
    foreground->input = input;
    foreground->focus = focus;

    // create members
    foreground->codes = codes_new(&config->codes);
    foreground->overlay = overlay_new(&config->overlay);
    foreground->registry = registry_new(&config->control);

    return foreground;
}

void foreground_destroy(Foreground *foreground)
{
    // free members
    codes_destroy(foreground->codes);
    overlay_destroy(foreground->overlay);
    registry_destroy(foreground->registry);

    // free control management
    g_hash_table_unref(foreground->controls_to_tags);

    // free main loop
    g_main_loop_unref(foreground->loop);

    g_free(foreground);
}

void foreground_run(Foreground *foreground)
{
    if (foreground_is_running(foreground))
        return;

    // get active window
    AtspiAccessible *window = focus_get_window(foreground->focus);
    if (!window)
    {
        g_warning("foreground: No active window, stopping.");
        return;
    }

    // let the registry watch the window
    registry_watch(foreground->registry, window, (RegistrySubscriber){
                                                     .add = callback_control_add,
                                                     .remove = callback_control_remove,
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

    // clean up members
    registry_unwatch(foreground->registry);
    overlay_hide(foreground->overlay);
    g_object_unref(window);
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
    Foreground *foreground = foreground_ptr;

    // create the tag
    Tag *tag = codes_allocate(foreground->codes);
    g_hash_table_insert(foreground->controls_to_tags, control, tag);

    // configure the tag
    tag_set_config(tag, control_tag_config(control));

    // add to the overlay
    overlay_add(foreground->overlay, tag);

    //g_message("control added (code=%d)", g_array_index(control->code, guint, 0));
}

static void callback_control_remove(Control *control, gpointer foreground_ptr)
{
    Foreground *foreground = foreground_ptr;

    // get the tag
    Tag *tag = g_hash_table_lookup(foreground->controls_to_tags, control);
    g_hash_table_remove(foreground->controls_to_tags, control);

    // remove from overlay
    overlay_remove(foreground->overlay, tag);

    // deallocate tag
    codes_deallocate(foreground->codes, tag);

    //g_message("control removed");
}

static InputResponse callback_keyboard(InputEvent event, gpointer foreground_ptr)
{
    Foreground *foreground = foreground_ptr;

    // only bother with press events
    if (event.type != INPUT_KEY_PRESSED)
        return INPUT_CONSUME_EVENT;

    // process key type
    switch (event.id)
    {
    case GDK_KEY_Escape:
        foreground_quit(foreground);
        break;
    case GDK_KEY_BackSpace:
        codes_pop_key(foreground->codes);
        check_tag_matched(foreground);
        break;
    default:
        codes_add_key(foreground->codes, event.id);
        check_tag_matched(foreground);
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
    foreground_quit(foreground_ptr);
}

static void check_tag_matched(Foreground *foreground)
{
    // check to see if a tag was matched
    Tag *tag = codes_matching_tag(foreground->codes);
    if (!tag)
        return;

    // find the paired control
    Control *control = NULL;
    GHashTableIter iter;
    gpointer control_ptr, tag_ptr;
    g_hash_table_iter_init(&iter, foreground->controls_to_tags);
    while (g_hash_table_iter_next(&iter, &control_ptr, &tag_ptr))
        if (tag_ptr == tag)
            control = control_ptr;
    if (!control)
        return;

    // execute the matched control
    control_execute(control);

    // quit
    foreground_quit(foreground);
}
