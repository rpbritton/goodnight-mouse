/**
 * Copyright (C) 2021 Ryan Britton
 *
 * This file is part of goodnight_mouse.
 *
 * goodnight_mouse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * goodnight_mouse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with goodnight_mouse.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef B2D7397E_F9AC_4ACF_BCF8_B6487D76BF5B
#define B2D7397E_F9AC_4ACF_BCF8_B6487D76BF5B

#include <glib.h>

#include "overlay_config.h"
#include "codes_config.h"

// configuration for a foreground
typedef struct ForegroundConfig
{
    OverlayConfig *overlay;
    CodesConfig *codes;
} ForegroundConfig;

ForegroundConfig *foreground_new_config(GKeyFile *key_file);
void foreground_destroy_config(ForegroundConfig *config);

#endif /* B2D7397E_F9AC_4ACF_BCF8_B6487D76BF5B */
