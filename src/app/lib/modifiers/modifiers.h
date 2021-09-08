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

#ifndef DA946D1E_BC44_472D_8E6D_2165617DE963
#define DA946D1E_BC44_472D_8E6D_2165617DE963

#include <glib.h>

#include "../backend/backend.h"

// a window modifiers
typedef struct Modifiers
{
    Backend *backend;
} Modifiers;

Modifiers *modifiers_new(Backend *backend);
void modifiers_destroy(Modifiers *modifiers);
guint modifiers_get(Modifiers *modifiers);
guint modifiers_map(Modifiers *modifiers, guint mods);

#endif /* DA946D1E_BC44_472D_8E6D_2165617DE963 */
