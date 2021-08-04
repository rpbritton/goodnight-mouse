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

#include "executor.h"

#include <gdk/gdk.h>

#include "identify.h"

static gboolean execute_action(Executor *executor, AtspiAccessible *accessible, guint index);
static gboolean execute_modifiers(Executor *executor, guint modifiers, gboolean lock);
static gboolean execute_key(Executor *executor, guint key);
static gboolean execute_mouse(Executor *executor, AtspiAccessible *accessible, guint button);
static gboolean execute_focus(Executor *executor, AtspiAccessible *accessible);
static gboolean execute_press(Executor *executor, AtspiAccessible *accessible);

// creates a new executor
Executor *executor_new(Mouse *mouse, Keyboard *keyboard)
{
    Executor *executor = g_new(Executor, 1);

    // add members
    executor->mouse = mouse;
    executor->keyboard = keyboard;

    return executor;
}

// destroys an executor
void executor_destroy(Executor *executor)
{
    g_free(executor);
}

// executes an accessible by identifying it's control type, and potentially it's shifted variant
void executor_do(Executor *executor, AtspiAccessible *accessible, gboolean shifted)
{
    // get control type
    ControlType control_type = identify_control(accessible);

    // todo: figure out how to unset shift if shifted

    // choose action of executor
    switch (control_type)
    {
    case CONTROL_TYPE_NONE:
        break;

    case CONTROL_TYPE_PRESS:
        execute_press(executor, accessible);
        break;

    case CONTROL_TYPE_FOCUS:
        execute_focus(executor, accessible);
        break;

    case CONTROL_TYPE_TAB:
        if (!shifted)
            execute_press(executor, accessible);
        else
            execute_mouse(executor, accessible, 2);

        break;

    case CONTROL_TYPE_LINK:
        if (!shifted)
        {
            execute_press(executor, accessible);
        }
        else
        {
            // todo: doesn't do exactly what I want since shift is active

            // attempt using ctrl + return key
            if (execute_modifiers(executor, GDK_CONTROL_MASK, TRUE) &&
                execute_focus(executor, accessible) &&
                execute_key(executor, GDK_KEY_Return) &&
                execute_modifiers(executor, GDK_CONTROL_MASK, FALSE))
                break;

            // attempt using ctrl + mouse click
            if (execute_modifiers(executor, GDK_CONTROL_MASK, TRUE) &&
                execute_mouse(executor, accessible, 1) &&
                execute_modifiers(executor, GDK_CONTROL_MASK, FALSE))
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
            execute_press(executor, accessible);
        else
            execute_focus(executor, accessible);

        break;
    }
}

// execute the given accessible's action
static gboolean execute_action(Executor *executor, AtspiAccessible *accessible, guint index)
{
    g_debug("executor: Attempting action '%d'", index);

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
static gboolean execute_modifiers(Executor *executor, guint modifiers, gboolean lock)
{
    if (lock)
        g_debug("executor: Locking modifiers '%d'", modifiers);
    else
        g_debug("executor: Unlocking modifiers '%d'", modifiers);

    // attempt modifier set
    return keyboard_set_modifiers(executor->keyboard, modifiers, lock);
}

// presses and releases the given key
static gboolean execute_key(Executor *executor, guint key)
{
    g_debug("executor: Pressing key '%d'", key);

    // attempt key press
    return keyboard_press_key(executor->keyboard, key);
}

// executes a mouse click of the button into the center of the given accessible
static gboolean execute_mouse(Executor *executor, AtspiAccessible *accessible, guint button)
{
    g_debug("executor: Clicking mouse '%d'", button);

    // get position of the center of the accessible
    AtspiComponent *component = atspi_accessible_get_component_iface(accessible);
    if (!component)
        return FALSE;
    AtspiRect *bounds = atspi_component_get_extents(component, ATSPI_COORD_TYPE_SCREEN, NULL);
    gint x = bounds->x + bounds->width / 2;
    gint y = bounds->y + bounds->height / 2;
    g_object_unref(component);
    g_free(bounds);

    // attempt mouse press
    return mouse_press(executor->mouse, x, y, button);
}

// grabs the input focus onto the given accessible
static gboolean execute_focus(Executor *executor, AtspiAccessible *accessible)
{
    g_debug("executor: Focusing accessible");

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
static gboolean execute_press(Executor *executor, AtspiAccessible *accessible)
{
    // attempt press using action
    if (execute_action(executor, accessible, 0))
        return TRUE;

    // attempt press using return key
    if (execute_focus(executor, accessible) &&
        execute_key(executor, GDK_KEY_Return))
        return TRUE;

    // attempt press using mouse click
    if (execute_mouse(executor, accessible, 1))
        return TRUE;

    return FALSE;
}
