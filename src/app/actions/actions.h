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

typedef struct Actions
{
    Focus *focus;

    GSList *subscribers;
} Actions;

typedef void (*ActionsCallback)(GHashTable *list, gpointer data);

Actions *actions_new(Focus *focus);
void actions_destroy(Actions *actions);
void actions_subscribe(Actions *actions, ActionsCallback callback, gpointer data);
void actions_unsubscribe(Actions *actions, ActionsCallback callback);
GList *actions_list(Actions *actions);

#endif /* FE2ED0B7_0D51_459D_933A_9C5B78C8E618 */
