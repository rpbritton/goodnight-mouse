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

#if USE_X11

#include "utils.h"

#include <glib.h>
#include <X11/extensions/XInput2.h>

// get a window property.
// return NULL if error.
// must call XFree on returned value.
unsigned char *get_window_property(Display *display, Window window,
                                   const char *window_property, Atom req_type)
{
    // get window property atom
    Atom atom = XInternAtom(display, window_property, TRUE);

    // query window property
    int status;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *data;
    status = XGetWindowProperty(display, window, atom,
                                0, 1,
                                FALSE, req_type,
                                &actual_type, &actual_format,
                                &nitems, &bytes_after, &data);
    if (status != Success || !data)
        return NULL;

    return data;
}

// get the first device id of the given type. returns -1 if not found
int get_device_id(Display *display, int device_type)
{
    // get device id query from type
    int device_id;
    if (device_type == XIMasterKeyboard || device_type == XIMasterPointer)
        device_id = XIAllMasterDevices;
    else
        device_id = XIAllDevices;

    // query devices
    int num_devices;
    XIDeviceInfo *device_info = XIQueryDevice(display, device_id, &num_devices);

    // find device id
    device_id = -1;
    for (int index = 0; index < num_devices; index++)
    {
        if (device_info[index].use != device_type)
            continue;

        device_id = device_info[index].deviceid;
        break;
    }

    // free device into
    XIFreeDeviceInfo(device_info);

    // return device id
    return device_id;
}

#endif /* USE_X11 */
