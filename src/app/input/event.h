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

#ifndef E34E90F1_4A14_4140_9689_E5590AD1C2FE
#define E34E90F1_4A14_4140_9689_E5590AD1C2FE

#include <gdk/gdk.h>
#include <atspi/atspi.h>

#define INPUT_ALL_TYPES 0xFFFFFFFF
#define INPUT_ALL_IDS 0xFFFFFFFF
#define INPUT_ALL_MODIFIERS 0xFFFFFFFF

typedef enum InputEventType
{
    INPUT_KEY_PRESSED = (1 << ATSPI_KEY_PRESSED_EVENT),
    INPUT_KEY_RELEASED = (1 << ATSPI_KEY_RELEASED_EVENT),
    INPUT_BUTTON_PRESSED = (1 << ATSPI_BUTTON_PRESSED_EVENT),
    INPUT_BUTTON_RELEASED = (1 << ATSPI_BUTTON_RELEASED_EVENT),
} InputEventType;

typedef struct InputEvent
{
    InputEventType type;
    guint id;
    guint modifiers;
} InputEvent;

typedef enum InputResponse
{
    INPUT_RELAY_EVENT = FALSE,
    INPUT_CONSUME_EVENT = TRUE,
} InputResponse;

typedef InputResponse (*InputCallback)(InputEvent event, gpointer data);

#endif /* E34E90F1_4A14_4140_9689_E5590AD1C2FE */
