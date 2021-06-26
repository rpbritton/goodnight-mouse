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

#include "controller.h"

#include "identify.h"

#define EVENT_CHILD_ADD "object:children-changed:add"
#define EVENT_CHILD_REMOVE "object:children-changed:remove"

typedef struct Subscriber
{
    ControllerCallback callback;
    gpointer data;
} Subscriber;

static gint subscriber_matches_callback(gconstpointer subscriber, gconstpointer source);

static GList *recursively_get_controls(Controller *controller, AtspiAccessible *accessible);

static void callback_focus(AtspiAccessible *window, gpointer controller_ptr);
//static void callback_child_document_load(AtspiEvent *event, gpointer controller_ptr);
static void callback_child_add(AtspiEvent *event, gpointer controller_ptr);
static void callback_child_remove(AtspiEvent *event, gpointer controller_ptr);
// window deleted event?

Controller *controller_new(Focus *focus)
{
    Controller *controller = g_new(Controller, 1);

    controller->focus = focus;

    // init subscriber list
    controller->subscribers = NULL;

    // create match rules
    AtspiStateSet *interactive_states = atspi_state_set_new(NULL);
    atspi_state_set_add(interactive_states, ATSPI_STATE_SHOWING);
    atspi_state_set_add(interactive_states, ATSPI_STATE_VISIBLE);
    atspi_state_set_add(interactive_states, ATSPI_STATE_ENABLED);
    atspi_state_set_add(interactive_states, ATSPI_STATE_SENSITIVE);
    controller->match_interactive = atspi_match_rule_new(interactive_states, ATSPI_Collection_MATCH_ALL,
                                                         NULL, ATSPI_Collection_MATCH_NONE,
                                                         NULL, ATSPI_Collection_MATCH_NONE,
                                                         NULL, ATSPI_Collection_MATCH_NONE,
                                                         FALSE);
    g_object_unref(interactive_states);

    // register listeners
    focus_subscribe(controller->focus, callback_focus, controller);

    controller->listener_child_add = atspi_event_listener_new(callback_child_add, controller, NULL);
    atspi_event_listener_register(controller->listener_child_add, EVENT_CHILD_ADD, NULL);

    controller->listener_child_remove = atspi_event_listener_new(callback_child_remove, controller, NULL);
    atspi_event_listener_register(controller->listener_child_remove, EVENT_CHILD_REMOVE, NULL);

    return controller;
}

void controller_destroy(Controller *controller)
{
    // deregister listeners
    focus_unsubscribe(controller->focus, callback_focus);

    atspi_event_listener_deregister(controller->listener_child_add, EVENT_CHILD_ADD, NULL);
    g_object_unref(controller->listener_child_add);

    atspi_event_listener_deregister(controller->listener_child_remove, EVENT_CHILD_REMOVE, NULL);
    g_object_unref(controller->listener_child_remove);

    // free match rules
    g_object_unref(controller->match_interactive);

    // free subscriber lists
    g_slist_free_full(controller->subscribers, g_free);

    g_free(controller);
}

void controller_subscribe(Controller *controller, ControllerCallback callback, gpointer data)
{
    // todo: implement
    g_warning("Controller subscription not implemented!");
    return;

    Subscriber *subscriber = g_new(Subscriber, 1);

    *subscriber = (Subscriber){
        .callback = callback,
        .data = data,
    };

    controller->subscribers = g_slist_prepend(controller->subscribers, subscriber);
}

void controller_unsubscribe(Controller *controller, ControllerCallback callback)
{
    GSList *subscriber_node;
    while ((subscriber_node = g_slist_find_custom(controller->subscribers, callback, subscriber_matches_callback)))
    {
        g_free(subscriber_node->data);
        controller->subscribers = g_slist_remove_all(controller->subscribers, subscriber_node->data);
    }
}

static gint subscriber_matches_callback(gconstpointer subscriber, gconstpointer callback)
{
    // return 0 = match
    return !(((Subscriber *)subscriber)->callback == callback);
}

static GList *recursively_get_controls(Controller *controller, AtspiAccessible *accessible)
{
    GList *list = NULL;

    // check if accessible is control
    ControlType control_type = control_identify_type(accessible);
    if (control_type != CONTROL_TYPE_NONE)
        list = g_list_append(list, accessible);

    // get collection
    AtspiCollection *collection = atspi_accessible_get_collection_iface(accessible);
    if (!collection)
    {
        g_warning("controller: Collection is NULL");
        return list;
    }

    // process children that are interactive
    GArray *children_interactive = atspi_collection_get_matches(collection,
                                                                controller->match_interactive,
                                                                ATSPI_Collection_SORT_ORDER_CANONICAL,
                                                                0, FALSE, NULL);
    for (gint index = 0; index < children_interactive->len; index++)
    {
        AtspiAccessible *child = g_array_index(children_interactive, AtspiAccessible *, index);

        list = g_list_concat(list, recursively_get_controls(controller, child));

        g_object_unref(child);
    }
    g_array_unref(children_interactive);
    g_object_unref(collection);

    return list;
}

GList *controller_list(Controller *controller)
{
    AtspiAccessible *window = focus_window(controller->focus);
    if (!window)
        return NULL;

    GList *list = recursively_get_controls(controller, window);

    g_object_unref(window);

    return list;
}

static void callback_focus(AtspiAccessible *window, gpointer controller_ptr)
{
    if (!window)
        return;

    g_object_unref(window);
}

static void callback_child_add(AtspiEvent *event, gpointer controller_ptr)
{
    //g_message("add: name of sender: %s, name of source: %s, detail 1: %d, detail 2: %d, num children: %d", atspi_accessible_get_name(event->sender, NULL), atspi_accessible_get_name(event->source, NULL), event->detail1, event->detail2, atspi_accessible_get_child_count(event->source, NULL));

    g_boxed_free(ATSPI_TYPE_EVENT, event);
}

static void callback_child_remove(AtspiEvent *event, gpointer controller_ptr)
{
    //g_message("removed: name of sender: %s, name of source: %s", atspi_accessible_get_name(event->sender, NULL), atspi_accessible_get_name(event->source, NULL));

    g_boxed_free(ATSPI_TYPE_EVENT, event);
}