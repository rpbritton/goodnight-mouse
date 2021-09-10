/**
 * Copyright (C) 2021 ryan
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

#if USE_X11

#include "modifiers.h"

BackendX11Modifiers *backend_x11_modifiers_new(BackendX11 *backend)
{
    BackendX11Modifiers *modifiers = g_new(BackendX11Modifiers, 1);

    // add backend
    modifiers->backend = backend;

    // add x11 display
    modifiers->display = backend_x11_get_display(backend);
    modifiers->root_window = XDefaultRootWindow(modifiers->display);

    // add pointer device id
    XIGetClientPointer(modifiers->display, None, &modifiers->device_id);

    // get the virtual mods spec
    //modifiers->virt_mods_map = XkbGetMap(modifiers->display, XkbVirtualModMapMask, XkbUseCoreKbd);

    return modifiers;
}

void backend_x11_modifiers_destroy(BackendX11Modifiers *modifiers)
{
    // free keymap
    //XkbFreeClientMap(modifiers->virt_mods_map, 0, TRUE);

    // free
    g_free(modifiers);
}

guint backend_x11_modifiers_get(BackendX11Modifiers *modifiers)
{
    // get modifier state
    Window root, child;
    double root_x, root_y, win_x, win_y;
    XIButtonState buttons;
    XIModifierState mods;
    XIGroupState group;
    XIQueryPointer(modifiers->display, modifiers->device_id, modifiers->root_window,
                   &root, &child, &root_x, &root_y, &win_x, &win_y, &buttons, &mods, &group);
    g_message("got mods %d", mods.effective);

    // map and return mods
    return backend_x11_modifiers_map(modifiers, mods.effective);
}

guint backend_x11_modifiers_map(BackendX11Modifiers *modifiers, guint mods)
{
    //g_message("initial mods %d", mods);
    //guint mapped_virt_mods = 0;
    //XkbVirtualModsToReal(modifiers->virt_mods_map, mods, &mapped_virt_mods);
    //g_message("virt mods: %d, final mods %d", mapped_virt_mods, (mods | mapped_virt_mods) & 0xFF);
    //return (mods | mapped_virt_mods) & 0xFF;
    return mods;
}

#endif /* USE_X11 */