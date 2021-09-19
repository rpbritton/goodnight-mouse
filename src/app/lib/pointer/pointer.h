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

#ifndef D27E51CD_B77E_415D_B0E2_8E3239D55A1E
#define D27E51CD_B77E_415D_B0E2_8E3239D55A1E

#include "../backend/backend.h"
#include "../keymap/keymap.h"

// how to respond to a pointer event
typedef enum PointerEventResponse
{
    POINTER_EVENT_RELAY,
    POINTER_EVENT_CONSUME,
} PointerEventResponse;

// event for when a button is pressed or released
typedef struct PointerEvent
{
    guint button;
    gboolean pressed;
    GdkModifierType modifiers;
} PointerEvent;

// callback type used to notify for pointer events
typedef PointerEventResponse (*PointerCallback)(PointerEvent event, gpointer data);

// used to subscribe to events emitted from a pointer
typedef struct Pointer
{
    BackendPointer *backend;

    Keymap *keymap;

    GList *subscribers;
} Pointer;

Pointer *pointer_new(Backend *backend, Keymap *keymap);
void pointer_destroy(Pointer *pointer);
void pointer_subscribe(Pointer *pointer, PointerCallback callback, gpointer data);
void pointer_unsubscribe(Pointer *pointer, PointerCallback callback, gpointer data);
void pointer_subscribe_button(Pointer *pointer, guint button, GdkModifierType modifiers, PointerCallback callback, gpointer data);
void pointer_unsubscribe_button(Pointer *pointer, guint button, GdkModifierType modifiers, PointerCallback callback, gpointer data);

#endif /* D27E51CD_B77E_415D_B0E2_8E3239D55A1E */
