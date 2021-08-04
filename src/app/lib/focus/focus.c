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

#include "focus.h"

#include "window.h"

#define WINDOW_ACTIVATE_EVENT "window:activate"
#define WINDOW_DEACTIVATE_EVENT "window:deactivate"

typedef struct Subscriber
{
    FocusCallback callback;
    gpointer data;
} Subscriber;

static void callback_activation(AtspiEvent *event, gpointer listener_ptr);
static void callback_deactivation(AtspiEvent *event, gpointer listener_ptr);
static void notify_subscribers(Focus *listener);
static gint compare_subscriber_to_callback(gconstpointer subscriber_ptr, gconstpointer callback_ptr);

// creates a new focus listener and starts the listening
Focus *focus_new()
{
    Focus *listener = g_new(Focus, 1);

    // init subscribers
    listener->subscribers = NULL;

    // get currently focused window
    listener->window = focus_get_window_fresh();

    // register listeners
    listener->listener_activation = atspi_event_listener_new(callback_activation, listener, NULL);
    atspi_event_listener_register(listener->listener_activation, WINDOW_ACTIVATE_EVENT, NULL);
    listener->listener_deactivation = atspi_event_listener_new(callback_deactivation, listener, NULL);
    atspi_event_listener_register(listener->listener_deactivation, WINDOW_DEACTIVATE_EVENT, NULL);

    return listener;
}

// destroys a focus listener and stops the listening
void focus_destroy(Focus *listener)
{
    // deregister listeners
    atspi_event_listener_deregister(listener->listener_activation, WINDOW_ACTIVATE_EVENT, NULL);
    g_object_unref(listener->listener_activation);
    atspi_event_listener_deregister(listener->listener_deactivation, WINDOW_DEACTIVATE_EVENT, NULL);
    g_object_unref(listener->listener_deactivation);

    // free subscribers
    g_list_free_full(listener->subscribers, g_free);

    // unref window
    if (listener->window)
        g_object_unref(listener->window);

    g_free(listener);
}

// subscribe a callback to focus events
void focus_subscribe(Focus *listener, FocusCallback callback, gpointer data)
{
    // don't add if subscribed
    if (g_list_find_custom(listener->subscribers, callback, compare_subscriber_to_callback))
        return;

    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;

    // add subscriber
    listener->subscribers = g_list_append(listener->subscribers, subscriber);
}

// remove a callback from the subscribers
void focus_unsubscribe(Focus *listener, FocusCallback callback)
{
    // find every instance of the callback and remove
    GList *link = NULL;
    while ((link = g_list_find_custom(listener->subscribers, callback, compare_subscriber_to_callback)))
        listener->subscribers = g_list_delete_link(listener->subscribers, link);
}

// get the currently tracked window in the focus listener
AtspiAccessible *focus_get_window(Focus *listener)
{
    if (listener->window)
        g_object_ref(listener->window);
    return listener->window;
}

// handles a window activation event
static void callback_activation(AtspiEvent *event, gpointer listener_ptr)
{
    Focus *listener = listener_ptr;

    // make sure active window is different
    if (listener->window == event->source)
    {
        g_boxed_free(ATSPI_TYPE_EVENT, event);
        return;
    }

    // set focused window
    if (listener->window)
        g_object_unref(listener->window);
    listener->window = g_object_ref(event->source);

    // notify the subscribers
    notify_subscribers(listener);

    // free event
    g_boxed_free(ATSPI_TYPE_EVENT, event);
}

// handles a window deactivation event
static void callback_deactivation(AtspiEvent *event, gpointer listener_ptr)
{
    Focus *listener = listener_ptr;

    // make sure deactivating window is the same
    if (listener->window != event->source)
    {
        g_boxed_free(ATSPI_TYPE_EVENT, event);
        return;
    }

    // set focused window
    if (listener->window)
        g_object_unref(listener->window);
    listener->window = NULL;

    // notify the subscribers
    notify_subscribers(listener);

    // free event
    g_boxed_free(ATSPI_TYPE_EVENT, event);
}

// send the focused window to all subscribers
static void notify_subscribers(Focus *listener)
{
    for (GList *link = listener->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;
        subscriber->callback(listener->window, subscriber->data);
    }
}

// check if a subscriber matches a callback, returning 0 if so
static gint compare_subscriber_to_callback(gconstpointer subscriber_ptr, gconstpointer callback_ptr)
{
    return !(((Subscriber *)subscriber_ptr)->callback == callback_ptr);
}
