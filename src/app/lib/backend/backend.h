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

#include "common.h"

#if USE_X11

#include "x11/backend.h"
#define backend_new backend_x11_new
#define backend_destroy backend_x11_destroy

#include "x11/focus.h"
#define backend_focus_new backend_x11_focus_new
#define backend_focus_destroy backend_x11_focus_destroy
#define backend_focus_get_window backend_x11_focus_get_window

#else

#include "legacy/backend.h"
#define backend_new backend_legacy_new
#define backend_destroy backend_legacy_destroy

#include "legacy/focus.h"
#define backend_focus_new backend_legacy_focus_new
#define backend_focus_destroy backend_legacy_focus_destroy
#define backend_focus_get_window backend_legacy_focus_get_window

#endif

#endif /* C9468050_1653_4C80_B8A8_A79F04F64BF7 */
