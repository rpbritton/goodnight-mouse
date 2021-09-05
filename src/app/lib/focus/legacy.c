/**
 * Copyright (C) 2021 ryan
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

#include "legacy.h"

#define WINDOW_ACTIVATE_EVENT "window:activate"
#define WINDOW_DEACTIVATE_EVENT "window:deactivate"

static void set_active_window(FocusLegacy *focus_legacy);
static void set_window(FocusLegacy *focus_legacy, AtspiAccessible *accessible);
static void callback_focus(AtspiEvent *event, gpointer focus_legacy_ptr);

// create a new legacy focus listener
FocusLegacy *focus_legacy_new(FocusCallback callback, gpointer data)
{
    FocusLegacy *focus_legacy = g_new(FocusLegacy, 1);

    // add callback
    focus_legacy->callback = callback;
    focus_legacy->data = data;

    // set active window
    focus_legacy->accessible = NULL;
    set_active_window(focus_legacy);

    // register listeners
    focus_legacy->listener = atspi_event_listener_new(callback_focus, focus_legacy, NULL);
    atspi_event_listener_register(focus_legacy->listener, WINDOW_ACTIVATE_EVENT, NULL);
    atspi_event_listener_register(focus_legacy->listener, WINDOW_DEACTIVATE_EVENT, NULL);

    return focus_legacy;
}

// destroy the focus listener
void focus_legacy_destroy(FocusLegacy *focus_legacy)
{
    // deregister listeners
    atspi_event_listener_deregister(focus_legacy->listener, WINDOW_ACTIVATE_EVENT, NULL);
    atspi_event_listener_deregister(focus_legacy->listener, WINDOW_DEACTIVATE_EVENT, NULL);
    g_object_unref(focus_legacy->listener);

    // free accessible
    if (focus_legacy->accessible)
        g_object_unref(focus_legacy->accessible);

    // free
    g_free(focus_legacy);
}

// get the currently cached focused window
AtspiAccessible *focus_legacy_get_window(FocusLegacy *focus_legacy)
{
    if (focus_legacy->accessible)
        g_object_ref(focus_legacy->accessible);
    return focus_legacy->accessible;
}

// get and set the active window
static void set_active_window(FocusLegacy *focus_legacy)
{
    // get the (only) desktop
    AtspiAccessible *desktop = atspi_get_desktop(0);

    // loop through all applications
    AtspiAccessible *accessible = NULL;
    gint num_applications = atspi_accessible_get_child_count(desktop, NULL);
    for (gint application_index = 0; application_index < num_applications; application_index++)
    {
        AtspiAccessible *application = atspi_accessible_get_child_at_index(desktop, application_index, NULL);
        if (!application)
            continue;

        // loop through all windows
        gint num_windows = atspi_accessible_get_child_count(application, NULL);
        for (gint window_index = 0; window_index < num_windows; window_index++)
        {
            AtspiAccessible *window = atspi_accessible_get_child_at_index(application, window_index, NULL);
            if (!window)
                continue;

            // check if window is active
            AtspiStateSet *state_set = atspi_accessible_get_state_set(window);
            if (atspi_state_set_contains(state_set, ATSPI_STATE_ACTIVE))
            {
                if (accessible)
                {
                    const gchar *active_name = atspi_accessible_get_name(accessible, NULL);
                    const gchar *other_name = atspi_accessible_get_name(window, NULL);
                    g_warning("More than one window says they have focus! Using '%s', not '%s'",
                              active_name, other_name);
                    g_free((gpointer)active_name);
                    g_free((gpointer)other_name);
                }
                else
                {
                    accessible = g_object_ref(window);
                }
            }

            g_object_unref(state_set);
            g_object_unref(window);
        }

        g_object_unref(application);
    }

    g_object_unref(desktop);

    // set the window
    set_window(focus_legacy, accessible);
    if (accessible)
        g_object_unref(accessible);
}

// set the focused window and notify the listeners
static void set_window(FocusLegacy *focus_legacy, AtspiAccessible *accessible)
{
    // do nothing if already set
    if (accessible == focus_legacy->accessible)
        return;

    // set the window
    if (focus_legacy->accessible)
        g_object_unref(focus_legacy->accessible);
    if (accessible)
        g_object_ref(accessible);
    focus_legacy->accessible = accessible;

    // notify subscriber
    if (!focus_legacy->callback)
        return;
    focus_legacy->callback(focus_legacy->accessible, focus_legacy->data);
}

// handles a window activation and deactivation event
static void callback_focus(AtspiEvent *event, gpointer focus_legacy_ptr)
{
    FocusLegacy *focus_legacy = focus_legacy_ptr;

    // set the active window
    set_active_window(focus_legacy);

    // free the event
    g_boxed_free(ATSPI_TYPE_EVENT, event);
}
