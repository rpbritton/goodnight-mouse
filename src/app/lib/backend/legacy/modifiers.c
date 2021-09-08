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

#include "modifiers.h"

BackendLegacyModifiers *backend_legacy_modifiers_new(BackendLegacy *backend)
{
    BackendLegacyModifiers *modifiers = g_new(BackendLegacyModifiers, 1);

    // add backend
    modifiers->backend = backend;

    // get keymap
    modifiers->keymap = gdk_keymap_get_for_display(gdk_display_get_default());

    return modifiers;
}

void backend_legacy_modifiers_destroy(BackendLegacyModifiers *modifiers)
{
    // free
    g_free(modifiers);
}

guint backend_legacy_modifiers_get(BackendLegacyModifiers *modifiers)
{
    guint mods = gdk_keymap_get_modifier_state(modifiers->keymap);
    return backend_legacy_modifiers_map(modifiers, mods);
}

guint backend_legacy_modifiers_map(BackendLegacyModifiers *modifiers, guint mods)
{
    gdk_keymap_map_virtual_modifiers(modifiers->keymap, &mods);
    return mods & 0xFF;
}
