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

#ifndef F7E29EBF_1B6F_4049_BF43_6024E4B60EAD
#define F7E29EBF_1B6F_4049_BF43_6024E4B60EAD

#include "event.h"

// used to subscribe to all events emitted from a mouse
typedef struct MouseListener
{
    AtspiDeviceListener *atspi_listener;

    MouseCallback callback;
    gpointer callback_data;
} MouseListener;

MouseListener *mouse_listener_new(MouseCallback callback, gpointer data);
void mouse_listener_destroy(MouseListener *listener);

#endif /* F7E29EBF_1B6F_4049_BF43_6024E4B60EAD */
