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

#if USE_XCB

#include "xcb/xcb.h"
#define backend_new backend_xcb_new
#define backend_destroy backend_xcb_destroy

#include "xcb/focus.h"
#define backend_focus_new backend_xcb_focus_new
#define backend_focus_destroy backend_xcb_focus_destroy
#define backend_focus_get_window backend_xcb_focus_get_window

#include "xcb/keyboard.h"
#define backend_keyboard_new backend_xcb_keyboard_new
#define backend_keyboard_destroy backend_xcb_keyboard_destroy
#define backend_keyboard_grab backend_xcb_keyboard_grab
#define backend_keyboard_ungrab backend_xcb_keyboard_ungrab
#define backend_keyboard_grab_key backend_xcb_keyboard_grab_key
#define backend_keyboard_ungrab_key backend_xcb_keyboard_ungrab_key

#include "xcb/state.h"
#define backend_state_new backend_xcb_state_new
#define backend_state_destroy backend_xcb_state_destroy
#define backend_state_current backend_xcb_state_current

#include "xcb/emulator.h"
#define backend_emulator_new backend_xcb_emulator_new
#define backend_emulator_destroy backend_xcb_emulator_destroy
#define backend_emulator_reset backend_xcb_emulator_reset
#define backend_emulator_state backend_xcb_emulator_state
#define backend_emulator_key backend_xcb_emulator_key

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
#define backend_keyboard_get_state backend_legacy_keyboard_get_state
#define backend_keyboard_set_state backend_legacy_keyboard_set_state
#define backend_keyboard_set_key backend_legacy_keyboard_set_key

#endif

// backend type names
typedef void Backend;
typedef void BackendFocus;
typedef void BackendKeyboard;
typedef void BackendState;
typedef void BackendEmulator;

#endif /* C9468050_1653_4C80_B8A8_A79F04F64BF7 */
