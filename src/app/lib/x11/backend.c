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

#include "backend.h"

#include <glib.h>

BackendX11 *backend_x11_new()
{
    BackendX11 *backend = g_new(BackendX11, 1);

    // open x connection
    backend->display = XOpenDisplay(NULL);

    // return
    return backend;
}

void backend_x11_destroy(BackendX11 *backend)
{
    // close display
    XCloseDisplay(backend->display);

    // free
    g_free(backend);
}

Display *backend_x11_get_display(BackendX11 *backend)
{
    return backend->display;
}

#endif /* USE_X11 */
