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

#ifndef F9726048_1986_47DC_B2EB_3C6D9C3101FE
#define F9726048_1986_47DC_B2EB_3C6D9C3101FE

#include <glib.h>
#include <atspi/atspi.h>

#include "event.h"

// input watcher that listens for key and mouse button events, and relays
// them onto subscribers
typedef struct Input
{
    GSList *subscribers;

    gboolean listening;
    AtspiDeviceListener *keyboard_listener;
    AtspiDeviceListener *mouse_listener;
} Input;

Input *input_new();
void input_destroy(Input *input);
void input_subscribe(Input *input, InputEvent event, InputCallback callback, gpointer data);
void input_unsubscribe(Input *input, InputCallback callback);
guint input_modifiers(Input *input);
void input_start(Input *input);
void input_stop(Input *input);

#endif /* F9726048_1986_47DC_B2EB_3C6D9C3101FE */
