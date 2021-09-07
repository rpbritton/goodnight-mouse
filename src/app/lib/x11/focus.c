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

#if USE_X11

#include "focus.h"

#include "utils.h"

static Window get_active_window(BackendX11Focus *focus);
static guint get_window_pid(BackendX11Focus *focus, Window window);
static void set_active_window(BackendX11Focus *focus);
static void set_window(BackendX11Focus *focus, AtspiAccessible *accessible);
static void callback_property_notify(XEvent *event, gpointer focus_ptr);

static const char *window_property_active_window = "_NET_ACTIVE_WINDOW";
static const char *window_property_window_pid = "_NET_WM_PID";

BackendX11Focus *backend_x11_focus_new(BackendX11 *backend, BackendX11FocusCallback callback, gpointer data)
{
    BackendX11Focus *focus = g_new(BackendX11Focus, 1);

    // add backend
    focus->backend = backend;

    // add callback
    focus->callback = callback;
    focus->data = data;

    // add x connection
    focus->display = backend_x11_get_display(focus->backend);
    focus->root_window = XDefaultRootWindow(focus->display);

    // set active window
    focus->accessible = NULL;
    set_active_window(focus);

    // subscribe to focus events
    backend_x11_subscribe(focus->backend, PropertyNotify, callback_property_notify, focus);

    // return
    return focus;
}

void backend_x11_focus_destroy(BackendX11Focus *focus)
{
    // unsubscribe to focus events
    backend_x11_unsubscribe(focus->backend, PropertyNotify, callback_property_notify, focus);

    // free accessible
    if (focus->accessible)
        g_object_unref(focus->accessible);

    // free
    g_free(focus);
}

AtspiAccessible *backend_x11_focus_get_window(BackendX11Focus *focus)
{
    if (focus->accessible)
        g_object_ref(focus->accessible);
    return focus->accessible;
}

static Window get_active_window(BackendX11Focus *focus)
{
    // get property
    unsigned char *data = get_window_property(focus->display, focus->root_window,
                                              window_property_active_window, XA_WINDOW);
    if (!data)
        return None;

    // parse window
    Window window = ((Window *)data)[0];
    XFree(data);
    return window;
}

static guint get_window_pid(BackendX11Focus *focus, Window window)
{
    // get property
    unsigned char *data = get_window_property(focus->display, window,
                                              window_property_window_pid, XA_CARDINAL);
    if (!data)
        return 0;

    // parse pid
    guint pid = ((guint *)data)[0];
    XFree(data);
    return pid;
}

static void set_active_window(BackendX11Focus *focus)
{
    // get active window
    Window active_window = get_active_window(focus);
    if (active_window == None)
    {
        // no window found
        set_window(focus, NULL);
        return;
    }

    // get active window pid
    guint pid = get_window_pid(focus, active_window);
    if (pid == 0)
    {
        // no application found found
        set_window(focus, NULL);
        return;
    }

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

        // only check application with matching pid
        if (atspi_accessible_get_process_id(application, NULL) != pid)
        {
            g_object_unref(application);
            continue;
        }

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

static void set_window(BackendX11Focus *focus, AtspiAccessible *accessible)
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

static void callback_property_notify(XEvent *event, gpointer focus_ptr)
{
    BackendX11Focus *focus = focus_ptr;

    //
}

#endif /* USE_X11 */
