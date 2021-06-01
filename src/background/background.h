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

#ifndef SRC_BACKGROUND_BACKGROUND_H
#define SRC_BACKGROUND_BACKGROUND_H

#include <glib.h>
#include <gdk/gdk.h>

#include "../input/input.h"
#include "../foreground/foreground.h"

#define BACKGROUND_DEFAULT_CONFIG            \
    {                                        \
        .trigger_event = {                   \
            .type = ATSPI_KEY_PRESSED_EVENT, \
            .id = GDK_KEY_v,                 \
            .modifiers = GDK_SUPER_MASK,     \
        },                                   \
    }

typedef struct BackgroundConfig
{
    InputEvent trigger_event;
} BackgroundConfig;

typedef struct Background
{
    GMainLoop *loop;

    Input *input;
    Foreground *foreground;

    InputEvent trigger_event;
} Background;

Background *background_new(Input *input, Foreground *foreground);
void background_destroy(Background *background);
void background_configure(Background *background, BackgroundConfig config);
void background_run(Background *background);
gboolean background_is_running(Background *background);
void background_quit(Background *background);

#endif /* SRC_BACKGROUND_BACKGROUND_H */
