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

#ifndef C9468050_1653_4C80_B8A8_A79F04F64BF7
#define C9468050_1653_4C80_B8A8_A79F04F64BF7

#if USE_X11

#include "x11/x11.h"
#define backend_new backend_x11_new
#define backend_destroy backend_x11_destroy

#include "x11/focus.h"
#define backend_focus_new backend_x11_focus_new
#define backend_focus_destroy backend_x11_focus_destroy
#define backend_focus_get_window backend_x11_focus_get_window

#include "x11/keyboard.h"
#define backend_keyboard_new backend_x11_keyboard_new
#define backend_keyboard_destroy backend_x11_keyboard_destroy
#define backend_keyboard_grab backend_x11_keyboard_grab
#define backend_keyboard_ungrab backend_x11_keyboard_ungrab
#define backend_keyboard_grab_key backend_x11_keyboard_grab_key
#define backend_keyboard_ungrab_key backend_x11_keyboard_ungrab_key

#include "x11/modifiers.h"
#define backend_modifiers_new backend_x11_modifiers_new
#define backend_modifiers_destroy backend_x11_modifiers_destroy
#define backend_modifiers_get backend_x11_modifiers_get
#define backend_modifiers_map backend_x11_modifiers_map

#else

#include "legacy/legacy.h"
#define backend_new backend_legacy_new
#define backend_destroy backend_legacy_destroy

#include "legacy/focus.h"
#define backend_focus_new backend_legacy_focus_new
#define backend_focus_destroy backend_legacy_focus_destroy
#define backend_focus_get_window backend_legacy_focus_get_window

#include "legacy/keyboard.h"
#define backend_keyboard_new backend_legacy_keyboard_new
#define backend_keyboard_destroy backend_legacy_keyboard_destroy
#define backend_keyboard_grab backend_legacy_keyboard_grab
#define backend_keyboard_ungrab backend_legacy_keyboard_ungrab
#define backend_keyboard_grab_key backend_legacy_keyboard_grab_key
#define backend_keyboard_ungrab_key backend_legacy_keyboard_ungrab_key

#include "legacy/modifiers.h"
#define backend_modifiers_new backend_legacy_modifiers_new
#define backend_modifiers_destroy backend_legacy_modifiers_destroy
#define backend_modifiers_get backend_legacy_modifiers_get
#define backend_modifiers_map backend_legacy_modifiers_map

#endif

// backend type names
typedef void Backend;
typedef void BackendFocus;
typedef void BackendKeyboard;
typedef void BackendModifiers;

#endif /* C9468050_1653_4C80_B8A8_A79F04F64BF7 */
