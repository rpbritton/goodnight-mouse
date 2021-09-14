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

#ifndef E3785A2E_2BB8_4BFB_820D_F8EF8D3EA467
#define E3785A2E_2BB8_4BFB_820D_F8EF8D3EA467

#if USE_XCB

#include <glib.h>
#include <xcb/xcb.h>

typedef gboolean (*XCBSourceCallback)(xcb_generic_event_t *event, gpointer data);

GSource *xcb_source_new(xcb_connection_t *connection);

#endif /* USE_XCB */

#endif /* E3785A2E_2BB8_4BFB_820D_F8EF8D3EA467 */
