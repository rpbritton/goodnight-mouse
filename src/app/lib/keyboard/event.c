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

#include "event.h"

#include "modifiers.h"

// convert an atspi device event into a keyboard event
KeyboardEvent keyboard_event_from_atspi(AtspiDeviceEvent *atspi_event)
{
    KeyboardEvent event = {
        .type = (1 << atspi_event->type),
        .key = atspi_event->id,
        .modifiers = keyboard_modifiers_map(atspi_event->modifiers),
    };
    return event;
}