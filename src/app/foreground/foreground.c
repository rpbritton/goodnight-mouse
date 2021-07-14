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

static void foreground_code_add(Foreground *foreground, guint key);
static void foreground_code_pop(Foreground *foreground);
static void foreground_code_check(Foreground *foreground);

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
    foreground->code = g_array_new(FALSE, FALSE, sizeof(guint));
    foreground->keys = g_array_ref(config->codes.keys);

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
    g_array_unref(foreground->code);
    g_array_unref(foreground->keys);

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

    // clear the current code
    g_array_remove_range(foreground->code, 0, foreground->code->len);

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

    // remove from overlay
    overlay_remove(foreground->overlay, tag);

    // remove reference
    g_hash_table_remove(foreground->controls_to_tags, control);

    // deallocate tag
    codes_deallocate(foreground->codes, tag);

    //g_message("control removed");
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
    case GDK_KEY_BackSpace:
        foreground_code_pop(foreground_ptr);
        break;
    default:
        foreground_code_add(foreground_ptr, event.id);
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

static void foreground_code_add(Foreground *foreground, guint key)
{
    // make sure key is valid
    gboolean key_exists = FALSE;
    for (gint index = 0; index < foreground->keys->len; index++)
        if (key == g_array_index(foreground->keys, guint, index))
            key_exists = TRUE;
    if (!key_exists)
        return;

    // add key
    foreground->code = g_array_append_val(foreground->code, key);

    // update tags
    foreground_code_check(foreground);
}

static void foreground_code_pop(Foreground *foreground)
{
    // make sure a key can be popped
    if (foreground->code->len == 0)
        return;

    // remove last key
    foreground->code = g_array_remove_index(foreground->code, foreground->code->len - 1);

    // update tags
    foreground_code_check(foreground);
}

static void foreground_code_check(Foreground *foreground)
{
    // do nothing if no controls exist
    if (g_hash_table_size(foreground->controls_to_tags) == 0)
        return;

    // check each tag for matching
    gboolean partial_matches = 0;
    GHashTableIter iter;
    gpointer control_ptr, tag_ptr;
    g_hash_table_iter_init(&iter, foreground->controls_to_tags);
    while (g_hash_table_iter_next(&iter, &control_ptr, &tag_ptr))
    {
        switch (tag_match_code(tag_ptr, foreground->code))
        {
        case TAG_MATCH:
            // tag matched; execute and exit
            control_execute(control_ptr);
            foreground_quit(foreground);
            return;
        case TAG_PARTIAL_MATCH:
            partial_matches = TRUE;
            break;
        case TAG_NO_MATCH:
            continue;
        }
    }

    // do nothing if partial matches exist
    if (partial_matches)
        return;

    // reset code if no matches
    foreground->code = g_array_remove_range(foreground->code, 0, foreground->code->len);
    foreground_code_check(foreground);
}