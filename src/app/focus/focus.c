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

#define WINDOW_ACTIVATE_EVENT "window:activate"
#define WINDOW_DEACTIVATE_EVENT "window:deactivate"

typedef struct Subscriber
{
    FocusCallback callback;
    gpointer data;
} Subscriber;

static gint subscriber_matches_callback(gconstpointer subscriber, gconstpointer source);
static void activation_callback(AtspiEvent *event, void *focus_ptr);
static void deactivation_callback(AtspiEvent *event, void *focus_ptr);
static void notify_subscribers(Focus *focus);
static AtspiAccessible *force_window();

Focus *focus_new()
{
    Focus *focus = g_new(Focus, 1);

    // initialize subscriber list
    focus->subscribers = NULL;

    // get first window
    focus->window = force_window(focus);

    // register listeners
    focus->listener_activation = atspi_event_listener_new(activation_callback, focus, NULL);
    atspi_event_listener_register(focus->listener_activation, WINDOW_ACTIVATE_EVENT, NULL);
    focus->listener_deactivation = atspi_event_listener_new(deactivation_callback, focus, NULL);
    atspi_event_listener_register(focus->listener_deactivation, WINDOW_DEACTIVATE_EVENT, NULL);

    return focus;
}

void focus_destroy(Focus *focus)
{
    // deregister listeners
    atspi_event_listener_deregister(focus->listener_activation, WINDOW_ACTIVATE_EVENT, NULL);
    g_object_unref(focus->listener_activation);
    atspi_event_listener_deregister(focus->listener_deactivation, WINDOW_DEACTIVATE_EVENT, NULL);
    g_object_unref(focus->listener_deactivation);

    // unref window
    g_object_unref(focus->window);

    // free subscriber lists
    g_slist_free_full(focus->subscribers, g_free);

    g_free(focus);
}

void focus_subscribe(Focus *focus, FocusCallback callback, gpointer data)
{
    Subscriber *subscriber = g_new(Subscriber, 1);

    *subscriber = (Subscriber){
        .callback = callback,
        .data = data,
    };

    focus->subscribers = g_slist_prepend(focus->subscribers, subscriber);
}

void focus_unsubscribe(Focus *focus, FocusCallback callback)
{
    GSList *subscriber_node;
    while ((subscriber_node = g_slist_find_custom(focus->subscribers, callback, subscriber_matches_callback)))
    {
        g_free(subscriber_node->data);
        focus->subscribers = g_slist_remove_all(focus->subscribers, subscriber_node->data);
    }
}

static gint subscriber_matches_callback(gconstpointer subscriber, gconstpointer callback)
{
    // return 0 = match
    return !(((Subscriber *)subscriber)->callback == callback);
}

static void activation_callback(AtspiEvent *event, void *focus_ptr)
{
    Focus *focus = (Focus *)focus_ptr;

    // set focused window
    if (focus->window)
        g_object_unref(focus->window);
    focus->window = g_object_ref(event->source);

    // notify subscribers
    notify_subscribers(focus);

    const gchar *active_name = atspi_accessible_get_name(focus->window, NULL);
    g_debug("focus: Activated window '%s'", active_name);
    g_free((void *)active_name);

    g_boxed_free(ATSPI_TYPE_EVENT, event);
}

static void deactivation_callback(AtspiEvent *event, void *focus_ptr)
{
    Focus *focus = (Focus *)focus_ptr;

    // check to make sure we should be removing the currently focused window
    if (focus->window != event->source)
    {
        g_debug("focus: Deactivated inactive window");
        g_boxed_free(ATSPI_TYPE_EVENT, event);
        return;
    }

    // set focused window
    if (focus->window)
        g_object_unref(focus->window);
    focus->window = NULL;

    // notify subscribers
    notify_subscribers(focus);

    g_debug("focus: Deactivated window");

    g_boxed_free(ATSPI_TYPE_EVENT, event);
}

static void notify_subscribers(Focus *focus)
{
    for (GSList *sub = focus->subscribers; sub; sub = sub->next)
    {
        if (focus->window)
            g_object_ref(focus->window);

        Subscriber *subscriber = (Subscriber *)sub->data;
        subscriber->callback(focus->window, subscriber->data);
    }
}

AtspiAccessible *focus_window(Focus *focus)
{
    if (focus->window)
        g_object_ref(focus->window);

    return focus->window;
}

static AtspiAccessible *force_window()
{
    AtspiAccessible *active_window = NULL;

    // get the (only) desktop
    AtspiAccessible *desktop = atspi_get_desktop(0);

    // todo: consider alternative approach (e.g. X11 libraries; it's faster)

    // loop through all applications
    gint num_applications = atspi_accessible_get_child_count(desktop, NULL);
    for (gint application_index = 0; application_index < num_applications; application_index++)
    {
        AtspiAccessible *application = atspi_accessible_get_child_at_index(desktop, application_index, NULL);
        if (!application)
            continue;

        // loop through all windows
        gint num_windows = atspi_accessible_get_child_count(application, NULL);
        for (gint window_index = 0; window_index < num_windows; window_index++)
        {
            AtspiAccessible *window = atspi_accessible_get_child_at_index(application, window_index, NULL);
            if (!window)
                continue;

            // check if window is active
            AtspiStateSet *state_set = atspi_accessible_get_state_set(window);
            if (atspi_state_set_contains(state_set, ATSPI_STATE_ACTIVE))
            {
                if (active_window)
                {
                    const gchar *active_name = atspi_accessible_get_name(active_window, NULL);
                    const gchar *other_name = atspi_accessible_get_name(window, NULL);
                    g_warning("More than one window says they have focus! Using '%s', not '%s'",
                              active_name, other_name);
                    g_free((void *)active_name);
                    g_free((void *)other_name);
                }
                else
                {
                    active_window = g_object_ref(window);
                }
            }

            g_object_unref(state_set);
            g_object_unref(window);
        }

        g_object_unref(application);
    }

    g_object_unref(desktop);

    return active_window;
}