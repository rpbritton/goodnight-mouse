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

#ifndef F57F1019_9CFE_4F5D_A723_17B8C671BC05
#define F57F1019_9CFE_4F5D_A723_17B8C671BC05

#include <atspi/atspi.h>

#include "legacy.h"
#include "../common/keyboard.h"

// backend for keyboard events that uses pure atspi
typedef struct BackendLegacyKeyboard
{
    BackendLegacy *backend;

    BackendKeyboardCallback callback;
    gpointer data;

    gboolean registered;
    AtspiDeviceListener *listener;

    gint grabs;
    GList *key_grabs;
} BackendLegacyKeyboard;

BackendLegacyKeyboard *backend_legacy_keyboard_new(BackendLegacy *backend, BackendKeyboardCallback callback, gpointer data);
void backend_legacy_keyboard_destroy(BackendLegacyKeyboard *keyboard);
void backend_legacy_keyboard_grab(BackendLegacyKeyboard *keyboard);
void backend_legacy_keyboard_ungrab(BackendLegacyKeyboard *keyboard);
void backend_legacy_keyboard_grab_key(BackendLegacyKeyboard *keyboard, BackendKeyboardEvent event);
void backend_legacy_keyboard_ungrab_key(BackendLegacyKeyboard *keyboard, BackendKeyboardEvent event);
BackendKeyboardState backend_legacy_keyboard_get_modifiers(BackendLegacyKeyboard *keyboard);

#endif /* F57F1019_9CFE_4F5D_A723_17B8C671BC05 */
