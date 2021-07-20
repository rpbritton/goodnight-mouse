/**
 * Copyright (C) 2021 Ryan Britton
 *
 * This file is part of goodnight_mouse.
 *
 * goodnight_mouse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * goodnight_mouse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with goodnight_mouse.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "window.h"

#ifdef USE_X11
#include "X11/Xatom.h"
#include "X11/Xlib.h"
#endif

// gets the currently focused window using atspi and/or X11 libraries
AtspiAccessible *window_get_focused()
{
    AtspiAccessible *active_window = NULL;

#ifdef USE_X11
    Display *display = XOpenDisplay(NULL);
    Window root_window = RootWindow(display, DefaultScreen(display));

    int status;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *data;

    // get active window
    Atom window_atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", TRUE);

    status = XGetWindowProperty(display, root_window, window_atom,
                                0, 1,
                                FALSE, XA_WINDOW,
                                &actual_type, &actual_format,
                                &nitems, &bytes_after, &data);
    if (status != Success || !data)
        return NULL;

    Window window = ((Window *)data)[0];
    XFree(data);

    // get pid
    Atom pid_atom = XInternAtom(display, "_NET_WM_PID", TRUE);

    status = XGetWindowProperty(display, window, pid_atom,
                                0, 1,
                                FALSE, XA_CARDINAL,
                                &actual_type, &actual_format,
                                &nitems, &bytes_after, &data);
    if (status != Success || !data)
        return NULL;

    guint pid = ((guint *)data)[0];
    XFree(data);

    // close display
    XCloseDisplay(display);
#endif

    // get the (only) desktop
    AtspiAccessible *desktop = atspi_get_desktop(0);

    // loop through all applications
    gint num_applications = atspi_accessible_get_child_count(desktop, NULL);
    for (gint application_index = 0; application_index < num_applications; application_index++)
    {
        AtspiAccessible *application = atspi_accessible_get_child_at_index(desktop, application_index, NULL);
        if (!application)
            continue;

#ifdef USE_X11
        if (atspi_accessible_get_process_id(application, NULL) != pid)
        {
            g_object_unref(application);
            continue;
        }
#endif

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
                if (active_window)
                {
                    const gchar *active_name = atspi_accessible_get_name(active_window, NULL);
                    const gchar *other_name = atspi_accessible_get_name(window, NULL);
                    g_warning("More than one window says they have focus! Using '%s', not '%s'",
                              active_name, other_name);
                    g_free((gpointer)active_name);
                    g_free((gpointer)other_name);
                }
                else
                {
                    active_window = g_object_ref(window);
                }
            }

            g_object_unref(state_set);
            g_object_unref(window);
        }

        g_object_unref(application);
    }

    g_object_unref(desktop);

    return active_window;
}