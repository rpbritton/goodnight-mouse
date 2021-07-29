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

#ifndef C771728F_10C2_4C46_86DE_E96D9E622166
#define C771728F_10C2_4C46_86DE_E96D9E622166

#include <glib.h>
#include <atspi/atspi.h>

// a callback for when the currently focused window changes, possibly to NULL
typedef void (*FocusCallback)(AtspiAccessible *window, gpointer data);

// a window focus listener
typedef struct FocusListener
{
    FocusCallback callback;
    gpointer callback_data;

    AtspiAccessible *window;

    AtspiEventListener *listener_activation;
    AtspiEventListener *listener_deactivation;
} FocusListener;

FocusListener *focus_listener_new(FocusCallback callback, gpointer data);
void focus_listener_destroy(FocusListener *listener);

#endif /* C771728F_10C2_4C46_86DE_E96D9E622166 */
