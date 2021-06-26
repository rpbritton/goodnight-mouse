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

#ifndef FE2ED0B7_0D51_459D_933A_9C5B78C8E618
#define FE2ED0B7_0D51_459D_933A_9C5B78C8E618

#include <glib.h>

#include "../focus/focus.h"
#include "control.h"

typedef struct Controller
{
    Focus *focus;

    GSList *subscribers;

    AtspiMatchRule *match_interactive;

    AtspiEventListener *listener_child_add;
    AtspiEventListener *listener_child_remove;
} Controller;

typedef void (*ControllerCallback)(GHashTable *list, gpointer data);

Controller *controller_new(Focus *focus);
void controller_destroy(Controller *controller);
void controller_subscribe(Controller *controller, ControllerCallback callback, gpointer data);
void controller_unsubscribe(Controller *controller, ControllerCallback callback);
GList *controller_list(Controller *controller);

#endif /* FE2ED0B7_0D51_459D_933A_9C5B78C8E618 */
