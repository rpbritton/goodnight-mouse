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

#include "legacy.h"

#include <glib.h>

// create a new legacy backend
BackendLegacy *backend_legacy_new()
{
    BackendLegacy *backend = g_new(BackendLegacy, 1);

    // return
    return backend;
}

// destroy a legacy backend
void backend_legacy_destroy(BackendLegacy *backend)
{
    // free
    g_free(backend);
}
