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
#include "cache.h"

#define EVENT_CHILD_ADD "object:children-changed:add"
#define EVENT_CHILD_REMOVE "object:children-changed:remove"

typedef struct Subscriber
{
    ControllerCallback callback;
    gpointer data;
} Subscriber;

static gint subscriber_matches_callback(gconstpointer subscriber, gconstpointer source);

static void callback_focus(AtspiAccessible *window, gpointer controller_ptr);
static void callback_child_document_load(AtspiEvent *event, gpointer controller_ptr);
static void callback_child_add(AtspiEvent *event, gpointer controller_ptr);
static void callback_child_remove(AtspiEvent *event, gpointer controller_ptr);
// window deleted event?

Controller *controller_new(Focus *focus)
{
    Controller *controller = g_new(Controller, 1);

    controller->focus = focus;

    // init subscriber list
    controller->subscribers = NULL;

    // init cache
    controller->cache = cache_new();

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

    // free subscriber lists
    g_slist_free_full(controller->subscribers, g_free);

    // free cache
    cache_destroy(controller->cache);

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

GArray *controller_list(Controller *controller)
{
    AtspiAccessible *window = focus_window(controller->focus);
    if (!window)
        return NULL;

    GArray *list = cache_list(controller->cache, window);

    g_object_unref(window);

    return list;
}

static void callback_focus(AtspiAccessible *window, gpointer controller_ptr)
{
    if (!window)
        return;

    Controller *controller = (Controller *)controller_ptr;

    cache_add_window(controller->cache, window);

    g_object_unref(window);
}

static void callback_child_add(AtspiEvent *event, gpointer controller_ptr)
{
    Controller *controller = (Controller *)controller_ptr;

    g_message("add: name of sender: %s, name of source: %s, detail 1: %d, detail 2: %d, num children: %d", atspi_accessible_get_name(event->sender, NULL), atspi_accessible_get_name(event->source, NULL), event->detail1, event->detail2, atspi_accessible_get_child_count(event->source, NULL));

    // get added child
    AtspiAccessible *child = atspi_accessible_get_child_at_index(event->source, event->detail1, NULL);

    // check if child is found, otherwise check parent
    if (child)
        cache_add(controller->cache, child);
    else
        cache_add(controller->cache, event->source);

    if (child)
        g_object_unref(child);
    g_boxed_free(ATSPI_TYPE_EVENT, event);
}

static void callback_child_remove(AtspiEvent *event, gpointer controller_ptr)
{
    g_message("removed: name of sender: %s, name of source: %s", atspi_accessible_get_name(event->sender, NULL), atspi_accessible_get_name(event->source, NULL));

    g_boxed_free(ATSPI_TYPE_EVENT, event);
}