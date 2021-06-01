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

#ifndef SRC_APP_INPUT_INPUT_H
#define SRC_APP_INPUT_INPUT_H

#include <glib.h>
#include <atspi/atspi.h>
#include <gdk/gdk.h>

typedef enum InputEventAction
{
    INPUT_RELAY_EVENT = FALSE,
    INPUT_CONSUME_EVENT = TRUE,
} InputEventAction;

typedef struct Input
{
    GSList *subscribers;

    AtspiDeviceListener *keyboard_listener;
    AtspiDeviceListener *mouse_listener;
} Input;

typedef struct InputEvent
{
    AtspiEventType type;
    guint id;
    GdkModifierType modifiers;
} InputEvent;

typedef InputEventAction (*InputCallback)(InputEvent event, gpointer user_data);

Input *input_new();
void input_destroy(Input *input);
void input_subscribe(Input *input, InputEvent event, InputCallback callback, gpointer user_data);
void input_subscribe_all(Input *input, InputCallback callback, gpointer user_data);
void input_unsubscribe(Input *input, InputCallback callback);

#endif /* SRC_APP_INPUT_INPUT_H */