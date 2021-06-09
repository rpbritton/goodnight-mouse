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

#include "actions.h"

typedef struct Subscriber
{
    ActionsCallback callback;
    gpointer data;
} Subscriber;

static gint subscriber_matches_callback(gconstpointer subscriber, gconstpointer source);
static GList *get_actions(AtspiAccessible *accessible);

#define NUM_VISIBLE_STATES 4

static const AtspiStateType VISIBLE_STATES[NUM_VISIBLE_STATES] = {
    ATSPI_STATE_SHOWING,
    ATSPI_STATE_VISIBLE,
    ATSPI_STATE_ENABLED,
    ATSPI_STATE_SENSITIVE,
};

Actions *actions_new(Focus *focus)
{
    Actions *actions = g_new(Actions, 1);

    actions->focus = focus;

    // initialize subscriber list
    focus->subscribers = NULL;

    return actions;
}

void actions_destroy(Actions *actions)
{
    // free subscriber lists
    g_slist_free_full(actions->subscribers, g_free);

    g_free(actions);
}

void actions_subscribe(Actions *actions, ActionsCallback callback, gpointer data)
{
    g_warning("Actions subscription not implemented!");

    Subscriber *subscriber = g_new(Subscriber, 1);

    *subscriber = (Subscriber){
        .callback = callback,
        .data = data,
    };

    actions->subscribers = g_slist_prepend(actions->subscribers, subscriber);
}

void actions_unsubscribe(Actions *actions, ActionsCallback callback)
{
    GSList *subscriber_node;
    while ((subscriber_node = g_slist_find_custom(actions->subscribers, callback, subscriber_matches_callback)))
    {
        g_free(subscriber_node->data);
        actions->subscribers = g_slist_remove_all(actions->subscribers, subscriber_node->data);
    }
}

static gint subscriber_matches_callback(gconstpointer subscriber, gconstpointer callback)
{
    // return 0 = match
    return !(((Subscriber *)subscriber)->callback == callback);
}

GList *actions_list(Actions *actions)
{
    AtspiAccessible *window = focus_window(actions->focus);

    GList *list = get_actions(window);

    g_object_unref(window);

    return list;
}

static GList *get_actions(AtspiAccessible *accessible)
{
    GList *list = NULL;

    //// make sure is visible
    //if (accessible_is_visible(accessible))
    //    return list;
    //
    //// add if accessible is actionable
    //if (accessible_is_actionable(accessible))
    //    list = g_list_append(list, g_object_ref(accessible));

    // check children
    gint num_children = atspi_accessible_get_child_count(accessible, NULL);
    for (gint child_index = 0; child_index < num_children; child_index++)
    {
        AtspiAccessible *child = atspi_accessible_get_child_at_index(accessible, child_index, NULL);
        if (!child)
            continue;

        list = g_list_concat(list, get_actions(child));

        g_object_unref(child);
    }

    return list;
}

//static gboolean accessible_is_visible(AtspiAccessible *accessible)
//{
//    AtspiStateSet *state_set = atspi_accessible_get_state_set(accessible);
//
//    for (gint state_index = 0; state_index < NUM_VISIBLE_STATES; state_index++)
//    {
//        if (atspi_state_set_contains(state_set, VISIBLE_STATES[state_index]))
//        {
//            return FALSE;
//        }
//    }
//
//    //g_object_unref(state_set);
//
//    return TRUE;
//}
//
//static gboolean accessible_is_actionable(AtspiAccessible *accessible)
//{
//    // this needs to be more adaptable, return wrapper struct with what the action is.
//    return FALSE;
//}