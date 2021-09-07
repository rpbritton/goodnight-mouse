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

typedef struct Subscriber
{
    FocusCallback callback;
    gpointer data;
} Subscriber;

static void callback_focus_changed(gpointer focus_ptr);

// creates a new focus focus and starts the listening
Focus *focus_new(Backend *backend)
{
    Focus *focus = g_new(Focus, 1);

    // init subscribers
    focus->subscribers = NULL;

    // add backend
    focus->backend = backend_focus_new(backend, callback_focus_changed, focus);

    // set the current window
    focus->accessible = backend_focus_get_window(focus->backend);

    return focus;
}

// destroys a focus and stops the listening
void focus_destroy(Focus *focus)
{
    // free backend
    backend_focus_destroy(focus->backend);

    // free subscribers
    g_list_free_full(focus->subscribers, g_free);

    // unref window
    if (focus->accessible)
        g_object_unref(focus->accessible);

    g_free(focus);
}

// subscribe a callback to focus events
void focus_subscribe(Focus *focus, FocusCallback callback, gpointer data)
{
    // create a new subscriber
    Subscriber *subscriber = g_new(Subscriber, 1);
    subscriber->callback = callback;
    subscriber->data = data;

    // add subscriber
    focus->subscribers = g_list_append(focus->subscribers, subscriber);
}

// remove a callback from the subscribers
void focus_unsubscribe(Focus *focus, FocusCallback callback, gpointer data)
{
    // remove the first matching subscriber
    for (GList *link = focus->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;

        // check if subscriber matches
        if (subscriber->callback == callback &&
            subscriber->data == data)
        {
            // remove subscriber
            g_free(subscriber);
            focus->subscribers = g_list_delete_link(focus->subscribers, link);
            return;
        }
    }
}

// get the currently focused window
AtspiAccessible *focus_get_window(Focus *focus)
{
    if (focus->accessible)
        g_object_ref(focus->accessible);
    return focus->accessible;
}

// set the focused window and send it to the subscribers
static void callback_focus_changed(gpointer focus_ptr)
{
    Focus *focus = focus_ptr;

    // get the current window
    AtspiAccessible *accessible = backend_focus_get_window(focus->backend);
    if (accessible == focus->accessible)
    {
        if (accessible)
            g_object_unref(accessible);
        return;
    }

    // set the new window
    if (focus->accessible)
        g_object_unref(focus->accessible);
    focus->accessible = accessible;

    // notify the subscribers
    for (GList *link = focus->subscribers; link; link = link->next)
    {
        Subscriber *subscriber = link->data;
        subscriber->callback(focus->accessible, subscriber->data);
    }
}
