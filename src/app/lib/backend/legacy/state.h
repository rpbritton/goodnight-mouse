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

#ifndef C8ADFD83_0AFF_46BD_99CC_DF87237AE4E0
#define C8ADFD83_0AFF_46BD_99CC_DF87237AE4E0

#include "legacy.h"
#include "../state.h"

// backend for getting state
typedef struct BackendLegacyState
{
    BackendLegacy *backend;
} BackendLegacyState;

BackendLegacyState *backend_legacy_state_new(BackendLegacy *backend);
void backend_legacy_state_destroy(BackendLegacyState *state);
BackendStateEvent backend_legacy_state_current(BackendLegacyState *state);

#endif /* C8ADFD83_0AFF_46BD_99CC_DF87237AE4E0 */
