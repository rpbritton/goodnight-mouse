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

#ifndef BD2D3A3A_EF81_4C26_A692_58D39FBCBA95
#define BD2D3A3A_EF81_4C26_A692_58D39FBCBA95

#include <glib.h>

#include "event.h"

// an input event subscriber that contains the event to subscribe to and
// callback information
typedef struct Subscriber
{
    InputEvent event;
    InputCallback callback;
    gpointer data;
} Subscriber;

Subscriber *subscriber_new(InputEvent event, InputCallback callback, gpointer data);
void subscriber_destroy(Subscriber *subscriber);
InputResponse subscriber_call(Subscriber *subscriber, InputEvent event);
gint subscriber_compare_event(Subscriber *subscriber, InputEvent event);
gint subscriber_compare_callback(gconstpointer subscriber, gconstpointer callback);

#endif /* BD2D3A3A_EF81_4C26_A692_58D39FBCBA95 */
