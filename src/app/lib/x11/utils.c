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

#include "utils.h"

#include <glib.h>

unsigned char *get_window_property(Display *display, Window window,
                                   const char *window_property, Atom req_type)
{
    Atom atom = XInternAtom(display, window_property, TRUE);
    int status;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *data;
    status = XGetWindowProperty(display, window, atom,
                                0, 1,
                                FALSE, req_type,
                                &actual_type, &actual_format,
                                &nitems, &bytes_after, &data);
    if (status != Success || !data)
        return NULL;

    return data;
}

#endif /* USE_X11 */
