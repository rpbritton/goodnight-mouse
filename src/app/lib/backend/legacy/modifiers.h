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

#ifndef A8059ECD_A36D_4711_8AD9_1B6C15406061
#define A8059ECD_A36D_4711_8AD9_1B6C15406061

#include <gdk/gdk.h>

#include "backend.h"

// backend for keyboard modifiers
typedef struct BackendLegacyModifiers
{
    BackendLegacy *backend;

    GdkKeymap *keymap;
} BackendLegacyModifiers;

BackendLegacyModifiers *backend_legacy_modifiers_new(BackendLegacy *backend);
void backend_legacy_modifiers_destroy(BackendLegacyModifiers *modifiers);
guint backend_legacy_modifiers_get(BackendLegacyModifiers *modifiers);
guint backend_legacy_modifiers_map(BackendLegacyModifiers *modifiers, guint mods);

#endif /* A8059ECD_A36D_4711_8AD9_1B6C15406061 */
