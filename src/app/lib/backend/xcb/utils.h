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

#ifndef CF028CC6_EE42_43EC_B0B2_09CCB44C709D
#define CF028CC6_EE42_43EC_B0B2_09CCB44C709D

#if USE_XCB

#include <glib.h>
#include <xcb/xcb.h>
#include <xcb/xinput.h>

xcb_input_device_id_t device_id_from_device_type(xcb_connection_t *connection, xcb_input_device_type_t device_type);

#endif /* USE_XCB */

#endif /* CF028CC6_EE42_43EC_B0B2_09CCB44C709D */
