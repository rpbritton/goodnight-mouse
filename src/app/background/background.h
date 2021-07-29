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

#ifndef E1E91E8D_508E_4E2D_91EE_C1A29C1291D8
#define E1E91E8D_508E_4E2D_91EE_C1A29C1291D8

#include <glib.h>
#include <gdk/gdk.h>

#include "background_config.h"
#include "../lib/keyboard/key.h"
#include "../foreground/foreground.h"

// background state that can run a g main loop and trigger a foreground
// when a hotkey is pressed
typedef struct Background
{
    GMainLoop *loop;

    Foreground *foreground;

    KeyboardEvent trigger_event;
} Background;

Background *background_new(BackgroundConfig *config, Foreground *foreground);
void background_destroy(Background *background);
void background_run(Background *background);
gboolean background_is_running(Background *background);
void background_quit(Background *background);

#endif /* E1E91E8D_508E_4E2D_91EE_C1A29C1291D8 */
