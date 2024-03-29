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

#ifndef B1E39E9E_46A9_450C_AE4B_72A95E00B143
#define B1E39E9E_46A9_450C_AE4B_72A95E00B143

#include <glib.h>

#include "app_config.h"

#include "lib/backend/backend.h"
#include "lib/keymap.h"
#include "lib/state.h"
#include "lib/emulator.h"
#include "lib/keyboard.h"
#include "lib/pointer.h"
#include "lib/focus.h"

#include "background/background.h"
#include "foreground/foreground.h"

// an app that manages the lifetime of a foreground and or background
typedef struct App
{
    guint signal_sigusr1;
    guint signal_sigusr2;
    guint signal_sighup;
    guint signal_sigint;
    guint signal_sigterm;

    Backend *backend;

    Keymap *keymap;
    State *state;
    Emulator *emulator;
    Keyboard *keyboard;
    Pointer *pointer;
    Focus *focus;

    Background *background;
    Foreground *foreground;
} App;

App *app_new(AppConfig *config);
void app_destroy(App *app);
void app_run(App *app);
void app_run_once(App *app);
gboolean app_is_running(App *app);
void app_quit(App *app);

#endif /* B1E39E9E_46A9_450C_AE4B_72A95E00B143 */
