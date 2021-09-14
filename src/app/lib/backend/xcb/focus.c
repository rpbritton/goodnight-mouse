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

#if USE_XCB

#include "focus.h"

#include <string.h>

#define WINDOW_PROPERTY_ACTIVE_WINDOW "_NET_ACTIVE_WINDOW"
#define WINDOW_PROPERTY_WINDOW_PID "_NET_WM_PID"

static xcb_atom_t get_atom(BackendXCBFocus *focus, const char *name);
static guint32 get_window_pid(BackendXCBFocus *focus, xcb_window_t window);
static void callback_property_notify(xcb_generic_event_t *generic_event, gpointer focus_ptr);

// create a new xcb based focus event backend
BackendXCBFocus *backend_xcb_focus_new(BackendXCB *backend, BackendFocusCallback callback, gpointer data)
{
    BackendXCBFocus *focus = g_new(BackendXCBFocus, 1);

    // add backend
    focus->backend = backend;

    // add callback
    focus->callback = callback;
    focus->data = data;

    // add connection
    focus->connection = backend_xcb_get_connection(focus->backend);
    focus->root = backend_xcb_get_root(focus->backend);

    // get the atoms
    focus->atom_active_window = get_atom(focus, WINDOW_PROPERTY_ACTIVE_WINDOW);
    focus->atom_window_pid = get_atom(focus, WINDOW_PROPERTY_WINDOW_PID);

    // subscribe to property change events
    backend_xcb_subscribe(focus->backend, XCB_PROPERTY_NOTIFY, callback_property_notify, focus);

    // return
    return focus;
}

// destroy the focus backend
void backend_xcb_focus_destroy(BackendXCBFocus *focus)
{
    // unsubscribe from property change events
    backend_xcb_unsubscribe(focus->backend, XCB_PROPERTY_NOTIFY, callback_property_notify, focus);

    // free
    g_free(focus);
}

// get the currently focused window
AtspiAccessible *backend_xcb_focus_get_window(BackendXCBFocus *focus)
{
    // get active window
    xcb_window_t active_window = backend_xcb_focus_get_xcb_window(focus);
    if (active_window == XCB_NONE)
        return NULL;

    // get active window pid
    guint32 pid = get_window_pid(focus, active_window);
    if (pid == XCB_NONE)
        return NULL;

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

    // return the window
    return accessible;
}

// get the active xcb window
xcb_window_t backend_xcb_focus_get_xcb_window(BackendXCBFocus *focus)
{
    // get the property
    xcb_get_property_cookie_t cookie = xcb_get_property(focus->connection, 0, focus->root, focus->atom_active_window, XCB_ATOM_WINDOW, 0, 1);
    xcb_generic_error_t *error;
    xcb_get_property_reply_t *reply = xcb_get_property_reply(focus->connection, cookie, &error);

    // check response
    if (error)
    {
        g_warning("backend-xcb: Get active window failed, error (%d)", error->error_code);
        free(error);
        return XCB_NONE;
    }
    if (!reply)
        return XCB_NONE;

    // parse the property
    gpointer value = xcb_get_property_value(reply);
    free(reply);
    return *(xcb_window_t *)value;
}

// get an atom from string
static xcb_atom_t get_atom(BackendXCBFocus *focus, const char *name)
{
    // get atom
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(focus->connection, 0, strlen(name), name);
    xcb_generic_error_t *error;
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(focus->connection, cookie, &error);

    // check for error
    if (error)
    {
        g_warning("backend-xcb: Get atom failed, error (%d)", error->error_code);
        free(error);
        return XCB_NONE;
    }
    if (!reply)
        return XCB_NONE;

    // free and return
    xcb_atom_t atom = reply->atom;
    free(reply);
    return atom;
}

// get the window's controlling process pid
static guint32 get_window_pid(BackendXCBFocus *focus, xcb_window_t window)
{
    // get the property
    xcb_get_property_cookie_t cookie = xcb_get_property(focus->connection, 0, window, focus->atom_window_pid, XCB_ATOM_CARDINAL, 0, 1);
    xcb_generic_error_t *error;
    xcb_get_property_reply_t *reply = xcb_get_property_reply(focus->connection, cookie, &error);

    // check response
    if (error)
    {
        g_warning("backend-xcb: Get window pid failed, error (%d)", error->error_code);
        free(error);
        return XCB_NONE;
    }
    if (!reply)
        return XCB_NONE;

    // parse the property
    gpointer value = xcb_get_property_value(reply);
    free(reply);
    return *(guint32 *)value;
}

// callback to handle focus change by checking the active window property
static void callback_property_notify(xcb_generic_event_t *generic_event, gpointer focus_ptr)
{
    BackendXCBFocus *focus = focus_ptr;

    // ensure this is a property notify event
    if (generic_event->response_type != XCB_PROPERTY_NOTIFY)
        return;
    xcb_property_notify_event_t *event = (xcb_property_notify_event_t *)generic_event;

    // ensure atom is active window
    if (event->atom != focus->atom_active_window)
        return;

    // notify callback
    focus->callback(focus->data);
}

#endif /* USE_XCB */
