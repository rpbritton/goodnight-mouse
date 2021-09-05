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

#ifndef D86F09E1_2490_4135_A2CE_ABFD20432EBA
#define D86F09E1_2490_4135_A2CE_ABFD20432EBA

#include <atspi/atspi.h>

#include "event.h"

// focus listener that uses built in at-spi tools to watch
typedef struct FocusLegacy
{
    FocusCallback callback;
    gpointer data;

    AtspiAccessible *accessible;
    AtspiEventListener *listener;
} FocusLegacy;

FocusLegacy *focus_legacy_new(FocusCallback callback, gpointer data);
void focus_legacy_destroy(FocusLegacy *focus_legacy);
AtspiAccessible *focus_legacy_get_window(FocusLegacy *focus_legacy);

#endif /* D86F09E1_2490_4135_A2CE_ABFD20432EBA */
