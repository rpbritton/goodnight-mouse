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
    case CONTROL_TYPE_PRESS:
        if (!shifted)
        {
            // attempt press using action
            if (execute_action(accessible, 0))
                break;

            // attempt press using return key
            if (execute_focus(accessible) &&
                execute_key(GDK_KEY_Return))
                break;

            // attempt press using mouse click
            if (execute_mouse(accessible, 1))
                break;
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

    case CONTROL_TYPE_FOCUS:
        // attempt focus
        execute_focus(accessible);
        break;

    case CONTROL_TYPE_PAGE_TAB:
        if (!shifted)
        {
            // attempt press using action
            if (execute_action(accessible, 0))
                break;

            // attempt press using return key
            if (execute_focus(accessible) &&
                execute_key(GDK_KEY_Return))
                break;

            // attempt press using mouse click
            if (execute_mouse(accessible, 1))
                break;
        }
        else
        {
            // use middle mouse button to close tab
            execute_mouse(accessible, 2);
        }
        break;

    case CONTROL_TYPE_ONLY_ACTION:
        // use action
        execute_action(accessible, 0);
        break;

    case CONTROL_TYPE_NONE:
        break;
    }

    // resume input
    input_start(input);
}

static gboolean execute_action(AtspiAccessible *accessible, guint index)
{
    g_debug("execute_action: Attempting action '%d'", index);

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
        g_debug("execute_modifiers: Locking modifiers '%d'", modifiers);
    else
        g_debug("execute_modifiers: Unlocking modifiers '%d'", modifiers);

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
    g_debug("execute_key: Pressing key '%d'", key);

    // send key
    if (!atspi_generate_keyboard_event(key, NULL, ATSPI_KEY_SYM, NULL))
        return FALSE;

    // success
    return TRUE;
}

// executes a mouse click of the button into the center of the given accessible
static gboolean execute_mouse(AtspiAccessible *accessible, guint button)
{
    g_debug("execute_mouse: Clicking mouse '%d'", button);

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
    g_debug("execute_focus: Focusing accessible");

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

    // success
    return TRUE;
}
