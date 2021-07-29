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

#ifndef CABA104F_07B2_475B_9F0E_E472190346A3
#define CABA104F_07B2_475B_9F0E_E472190346A3

#include "event.h"

// listener used to subscribe to a single key event
typedef struct KeyListener
{
    AtspiDeviceListener *atspi_listener;
    GArray *atspi_key;
    AtspiKeyEventMask atspi_type;
    AtspiKeyMaskType atspi_modifiers;

    KeyboardCallback callback;
    gpointer callback_data;
} KeyListener;

KeyListener *key_listener_new(KeyboardEvent event, KeyboardCallback callback, gpointer data);
void key_listener_destroy(KeyListener *listener);

#endif /* CABA104F_07B2_475B_9F0E_E472190346A3 */
