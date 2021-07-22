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

#include "execution.h"

#include <gdk/gdk.h>

#include "identify.h"

static gboolean execute_action(AtspiAccessible *accessible, guint index);
static gboolean execute_modifiers(guint modifiers, gboolean lock);
static gboolean execute_key(guint key);
static gboolean execute_mouse(AtspiAccessible *accessible, guint button);
static gboolean execute_focus(AtspiAccessible *accessible);
static gboolean execute_press(AtspiAccessible *accessible);

// executes an accessible by identifying it's control type, and potentially it's shifted variant
void execute_control(Input *input, AtspiAccessible *accessible, gboolean shifted)
{
    // get control type
    ControlType control_type = identify_control(accessible);

    // pause input
    input_stop(input);

    // todo: figure out how to unset shift if shifted

    // choose action of execution
    switch (control_type)
    {
    case CONTROL_TYPE_NONE:
        break;

    case CONTROL_TYPE_PRESS:
        execute_press(accessible);
        break;

    case CONTROL_TYPE_FOCUS:
        execute_focus(accessible);
        break;

    case CONTROL_TYPE_TAB:
        if (!shifted)
            execute_press(accessible);
        else
            execute_mouse(accessible, 2);

        break;

    case CONTROL_TYPE_LINK:
        if (!shifted)
        {
            execute_press(accessible);
        }
        else
        {
            // todo: doesn't do exactly what I want since shift is active

            // attempt using ctrl + return key
            if (execute_modifiers(GDK_CONTROL_MASK, TRUE) &&
                execute_focus(accessible) &&
                execute_key(GDK_KEY_Return) &&
                execute_modifiers(GDK_CONTROL_MASK, FALSE))
                break;

            // attempt using ctrl + mouse click
            if (execute_modifiers(GDK_CONTROL_MASK, TRUE) &&
                execute_mouse(accessible, 1) &&
                execute_modifiers(GDK_CONTROL_MASK, FALSE))
                break;
        }
        break;

    case CONTROL_TYPE_FOCUSABLE:
        // check if focusable accessible has an action
        AtspiAction *action = atspi_accessible_get_action_iface(accessible);
        gint n_actions = 0;
        if (action)
        {
            n_actions = atspi_action_get_n_actions(action, NULL);
            g_object_unref(action);
        }

        if (n_actions > 0)
            execute_press(accessible);
        else
            execute_focus(accessible);

        break;
    }

    // resume input
    input_start(input);
}

static gboolean execute_action(AtspiAccessible *accessible, guint index)
{
    g_debug("execution: Attempting action '%d'", index);

    // get action
    AtspiAction *action = atspi_accessible_get_action_iface(accessible);
    if (!action)
        return FALSE;

    // make sure there is an action
    gint num_actions = atspi_action_get_n_actions(action, NULL);
    if (num_actions < index)
        return FALSE;

    // do the action
    gboolean success = atspi_action_do_action(action, index, NULL);
    g_object_unref(action);
    if (!success)
        return FALSE;

    // success
    return TRUE;
}

// lock or unlock a mask of modifiers
static gboolean execute_modifiers(guint modifiers, gboolean lock)
{
    if (lock)
        g_debug("execution: Locking modifiers '%d'", modifiers);
    else
        g_debug("execution: Unlocking modifiers '%d'", modifiers);

    // set modifiers
    AtspiKeySynthType synth_type = lock ? ATSPI_KEY_LOCKMODIFIERS : ATSPI_KEY_UNLOCKMODIFIERS;
    if (!atspi_generate_keyboard_event(modifiers, NULL, synth_type, NULL))
        return FALSE;

    // success
    return TRUE;
}

// presses and releases the given key
static gboolean execute_key(guint key)
{
    g_debug("execution: Pressing key '%d'", key);

    // send key
    if (!atspi_generate_keyboard_event(key, NULL, ATSPI_KEY_SYM, NULL))
        return FALSE;

    // success
    return TRUE;
}

// executes a mouse click of the button into the center of the given accessible
static gboolean execute_mouse(AtspiAccessible *accessible, guint button)
{
    g_debug("execution: Clicking mouse '%d'", button);

    // record the current mouse position
    GdkSeat *seat = gdk_display_get_default_seat(gdk_display_get_default());
    GdkDevice *mouse = gdk_seat_get_pointer(seat);
    GdkWindow *window = gdk_display_get_default_group(gdk_display_get_default());
    gint original_x, original_y;
    gdk_window_get_device_position(window, mouse, &original_x, &original_y, NULL);

    // get position of the center of the accessible
    AtspiComponent *component = atspi_accessible_get_component_iface(accessible);
    if (!component)
        return FALSE;
    AtspiRect *bounds = atspi_component_get_extents(component, ATSPI_COORD_TYPE_SCREEN, NULL);
    gint accessible_x = bounds->x + bounds->width / 2;
    gint accessible_y = bounds->y + bounds->height / 2;
    g_object_unref(component);
    g_free(bounds);

    // move and click the mouse
    gchar *event_name = g_strdup_printf("b%dc", button);
    gboolean success = atspi_generate_mouse_event(accessible_x, accessible_y, event_name, NULL);
    g_free(event_name);
    if (!success)
        return FALSE;

    // move the mouse back to original position
    if (!atspi_generate_mouse_event(original_x, original_y, "abs", NULL))
        return FALSE;

    // success
    return TRUE;
}

// grabs the input focus onto the given accessible
static gboolean execute_focus(AtspiAccessible *accessible)
{
    g_debug("execution: Focusing accessible");

    // check for focusable state
    AtspiStateSet *states = atspi_accessible_get_state_set(accessible);
    gboolean focusable = atspi_state_set_contains(states, ATSPI_STATE_FOCUSABLE);
    g_object_unref(states);
    if (!focusable)
        return FALSE;

    // get component
    AtspiComponent *component = atspi_accessible_get_component_iface(accessible);
    if (!component)
        return FALSE;

    // grab focus
    gboolean success = atspi_component_grab_focus(component, NULL);
    g_object_unref(component);
    if (!success)
        return FALSE;

    // check if now contains focused state
    states = atspi_accessible_get_state_set(accessible);
    gboolean focused = atspi_state_set_contains(states, ATSPI_STATE_FOCUSED);
    g_object_unref(states);
    if (!focused)
        return FALSE;

    // success
    return TRUE;
}

// attempt to press an accessible, like lift-clicking with a mouse on a button
static gboolean execute_press(AtspiAccessible *accessible)
{
    // attempt press using action
    if (execute_action(accessible, 0))
        return TRUE;

    // attempt press using return key
    if (execute_focus(accessible) &&
        execute_key(GDK_KEY_Return))
        return TRUE;

    // attempt press using mouse click
    if (execute_mouse(accessible, 1))
        return TRUE;

    return FALSE;
}
