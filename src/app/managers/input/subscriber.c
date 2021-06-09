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

#include "subscriber.h"

#include "modifiers.h"

Subscriber *subscriber_new(InputEvent event, InputCallback callback, gpointer data)
{
    Subscriber *subscriber = g_new(Subscriber, 1);

    *subscriber = (Subscriber){
        .event = (InputEvent){
            .type = event.type,
            .id = event.id,
            .modifiers = (event.modifiers == INPUT_ALL_MODIFIERS) ? INPUT_ALL_MODIFIERS : modifiers_map(event.modifiers),
        },
        .callback = callback,
        .data = data,
    };

    return subscriber;
}

void subscriber_free(Subscriber *subscriber)
{
    g_free(subscriber);
}

InputResponse subscriber_call(Subscriber *subscriber, InputEvent event)
{
    return subscriber->callback(event, subscriber->data);
}

gboolean subscriber_matches_event(Subscriber *subscriber, InputEvent event)
{
    return (((subscriber->event.type == INPUT_ALL_TYPES) || (subscriber->event.type & event.type)) &&
            ((subscriber->event.id == INPUT_ALL_IDS) || (subscriber->event.id == event.id)) &&
            ((subscriber->event.modifiers == INPUT_ALL_MODIFIERS) || (subscriber->event.modifiers == event.modifiers)));
}

gboolean subscriber_matches_callback(Subscriber *subscriber, InputCallback callback)
{
    return (subscriber->callback == callback);
}