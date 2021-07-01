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
#include <atspi/atspi.h>

#include "control.h"

typedef void (*RegistryCallback)(Control *control, gpointer data);

typedef struct Registry
{
    RegistryCallback callback_add;
    RegistryCallback callback_remove;
    gpointer callback_data;

    AtspiMatchRule *match_interactive;

    AtspiAccessible *window;
    GHashTable *controls;
    GHashTable *controls_fresh;
} Registry;

Registry *registry_new(RegistryCallback add, RegistryCallback remove, gpointer data);
void registry_destroy(Registry *registry);
void registry_reset(Registry *registry);
void registry_watch(Registry *registry, AtspiAccessible *window);
guint registry_count(Registry *registry);

#endif /* FE2ED0B7_0D51_459D_933A_9C5B78C8E618 */
