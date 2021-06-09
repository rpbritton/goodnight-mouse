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

#ifndef SRC_APP_FOREGROUND_FOREGROUND_H
#define SRC_APP_FOREGROUND_FOREGROUND_H

#include <glib.h>

#include "../subscriptions/input/input.h"
#include "../subscriptions/focus/focus.h"
#include "../subscriptions/accessibles/accessibles.h"

typedef struct ForegroundConfig
{
} ForegroundConfig;

typedef struct Foreground
{
    GMainLoop *loop;

    Input *input;
    Focus *focus;
    Accessibles *accessibles;
} Foreground;

Foreground *foreground_new(Input *input, Focus *focus, Accessibles *accessibles);
void foreground_destroy(Foreground *foreground);
void foreground_configure(Foreground *foreground, ForegroundConfig config);
void foreground_run(Foreground *foreground);
gboolean foreground_is_running(Foreground *foreground);
void foreground_quit(Foreground *foreground);

#endif /* SRC_APP_FOREGROUND_FOREGROUND_H */
