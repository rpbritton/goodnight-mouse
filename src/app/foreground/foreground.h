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

#ifndef AD82229D_9BCD_4C49_AC37_47128F926D4E
#define AD82229D_9BCD_4C49_AC37_47128F926D4E

#include <glib.h>

#include "foreground_config.h"

#include "registry.h"
#include "codes.h"
#include "overlay.h"
#include "executor.h"

#include "../lib/state.h"
#include "../lib/emulator.h"
#include "../lib/keyboard.h"
#include "../lib/pointer.h"
#include "../lib/focus.h"

// a foreground which when run will show an overlay populated with tags with codes.
// key events will narrow down the codes, an when one code is focused on, that
// accessible will be executed.
typedef struct Foreground
{
    GMainLoop *loop;
    gboolean is_running;

    GHashTable *accessible_to_tag;

    gboolean shifted;

    State *state;
    Emulator *emulator;
    Keyboard *keyboard;
    Pointer *pointer;
    Focus *focus;

    Registry *registry;
    Codes *codes;
    Overlay *overlay;
    Executor *executor;
} Foreground;

Foreground *foreground_new(ForegroundConfig *config, State *state, Emulator *emulator,
                           Keyboard *keyboard, Pointer *pointer, Focus *focus);
void foreground_destroy(Foreground *foreground);
void foreground_run(Foreground *foreground);
void foreground_run_async(Foreground *foreground);
gboolean foreground_is_running(Foreground *foreground);
void foreground_quit(Foreground *foreground);

#endif /* AD82229D_9BCD_4C49_AC37_47128F926D4E */
