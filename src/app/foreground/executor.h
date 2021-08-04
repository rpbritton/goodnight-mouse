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

#ifndef DC8D1073_8C84_4BB1_9DF3_49B95D76178D
#define DC8D1073_8C84_4BB1_9DF3_49B95D76178D

#include <atspi/atspi.h>

#include "../lib/mouse/emulation.h"

typedef struct Executor
{
    Mouse *mouse;
    //Keyboard *keyboard;
} Executor;

Executor *executor_new(Mouse *mouse);
void executor_destroy(Executor *executor);
void executor_do(Executor *executor, AtspiAccessible *accessible, gboolean shifted);

#endif /* DC8D1073_8C84_4BB1_9DF3_49B95D76178D */
