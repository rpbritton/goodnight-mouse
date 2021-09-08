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
' * along with Goodnight Mouse.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "modifiers.h"

// creates a new modifiers manager
Modifiers *modifiers_new(Backend *backend)
{
    Modifiers *modifiers = g_new(Modifiers, 1);

    // add backend
    modifiers->backend = backend_modifiers_new(backend);

    return modifiers;
}

// destroys a modifiers manager
void modifiers_destroy(Modifiers *modifiers)
{
    // free backend
    backend_modifiers_destroy(modifiers->backend);

    // free
    g_free(modifiers);
}

guint modifiers_get(Modifiers *modifiers)
{
    return backend_modifiers_get(modifiers->backend);
}

guint modifiers_map(Modifiers *modifiers, guint mods)
{
    return backend_modifiers_map(modifiers->backend, mods);
}
