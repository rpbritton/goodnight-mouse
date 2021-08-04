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

#ifndef D102CB85_DF5A_44CB_80DC_B281855A12AB
#define D102CB85_DF5A_44CB_80DC_B281855A12AB

#include "event.h"

// used to subscribe to events emitted from a keyboard
typedef struct KeyboardListener
{
    GList *subscribers;

    AtspiDeviceListener *device_listener;
} KeyboardListener;

KeyboardListener *keyboard_listener_new();
void keyboard_listener_destroy(KeyboardListener *listener);
void keyboard_listener_subscribe(KeyboardListener *listener, KeyboardCallback callback, gpointer data);
void keyboard_listener_subscribe_key(KeyboardListener *listener, KeyboardEvent event, KeyboardCallback callback, gpointer data);
void keyboard_listener_unsubscribe(KeyboardListener *listener, KeyboardCallback callback);

#endif /* D102CB85_DF5A_44CB_80DC_B281855A12AB */
