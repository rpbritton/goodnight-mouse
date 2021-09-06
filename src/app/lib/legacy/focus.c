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

#include "focus.h"

#define WINDOW_ACTIVATE_EVENT "window:activate"
#define WINDOW_DEACTIVATE_EVENT "window:deactivate"

static void set_active_window(BackendLegacyFocus *focus);
static void set_window(BackendLegacyFocus *focus, AtspiAccessible *accessible);
static void callback_focus(AtspiEvent *event, gpointer focus_ptr);

// create a new legacy focus listener
BackendLegacyFocus *backend_legacy_focus_new(BackendLegacy *backend, BackendLegacyFocusCallback callback, gpointer data)
{
    BackendLegacyFocus *focus = g_new(BackendLegacyFocus, 1);

    // add backend
    focus->backend = backend;

    // add callback
    focus->callback = callback;
    focus->data = data;

    // set active window
    focus->accessible = NULL;
    set_active_window(focus);

    // register listeners
    focus->listener = atspi_event_listener_new(callback_focus, focus, NULL);
    atspi_event_listener_register(focus->listener, WINDOW_ACTIVATE_EVENT, NULL);
    atspi_event_listener_register(focus->listener, WINDOW_DEACTIVATE_EVENT, NULL);

    return focus;
}

// destroy the focus listener
void backend_legacy_focus_destroy(BackendLegacyFocus *focus)
{
    // deregister listeners
    atspi_event_listener_deregister(focus->listener, WINDOW_ACTIVATE_EVENT, NULL);
    atspi_event_listener_deregister(focus->listener, WINDOW_DEACTIVATE_EVENT, NULL);
    g_object_unref(focus->listener);

    // free accessible
    if (focus->accessible)
        g_object_unref(focus->accessible);

    // free
    g_free(focus);
}

// get the currently cached focused window
AtspiAccessible *backend_legacy_focus_get_window(BackendLegacyFocus *focus)
{
    if (focus->accessible)
        g_object_ref(focus->accessible);
    return focus->accessible;
}

// get and set the active window
static void set_active_window(BackendLegacyFocus *focus)
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
    set_window(focus, accessible);
    if (accessible)
        g_object_unref(accessible);
}

// set the focused window and notify the listeners
static void set_window(BackendLegacyFocus *focus, AtspiAccessible *accessible)
{
    // do nothing if already set
    if (accessible == focus->accessible)
        return;

    // set the window
    if (focus->accessible)
        g_object_unref(focus->accessible);
    if (accessible)
        g_object_ref(accessible);
    focus->accessible = accessible;

    // notify subscriber
    if (!focus->callback)
        return;
    focus->callback(focus->accessible, focus->data);
}

// handles a window activation and deactivation event
static void callback_focus(AtspiEvent *event, gpointer focus_ptr)
{
    BackendLegacyFocus *focus = focus_ptr;

    // set the active window
    set_active_window(focus);

    // free the event
    g_boxed_free(ATSPI_TYPE_EVENT, event);
}
