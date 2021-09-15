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

#if USE_XCB

#include "utils.h"

// get the first device id that matches the device type
xcb_input_device_id_t device_id_from_device_type(xcb_connection_t *connection, xcb_input_device_type_t device_type)
{
    // get input device
    xcb_input_device_t device = XCB_INPUT_DEVICE_ALL;
    if (device_type == XCB_INPUT_DEVICE_TYPE_MASTER_POINTER || device_type == XCB_INPUT_DEVICE_TYPE_MASTER_KEYBOARD)
        device = XCB_INPUT_DEVICE_ALL_MASTER;

    // send request
    xcb_input_xi_query_device_cookie_t cookie;
    cookie = xcb_input_xi_query_device(connection, device);

    // get response
    xcb_generic_error_t *error = NULL;
    xcb_input_xi_query_device_reply_t *reply;
    reply = xcb_input_xi_query_device_reply(connection, cookie, &error);

    // handle response
    if (error != NULL)
    {
        g_warning("backend-xcb: Failed to query device id: error: %d", error->error_code);
        free(error);
    }
    if (!reply)
        return XCB_NONE;

    // look for matching device type
    gboolean device_found = FALSE;
    xcb_input_device_id_t device_id;
    xcb_input_xi_device_info_iterator_t iterator = xcb_input_xi_query_device_infos_iterator(reply);
    for (; iterator.rem; xcb_input_xi_device_info_next(&iterator))
    {
        xcb_input_xi_device_info_t *device_info = iterator.data;
        if (device_info->type != device_type)
            continue;
        device_found = TRUE;
        device_id = device_info->deviceid;
        break;
    }
    free(reply);

    // ensure device id was found
    if (!device_found)
    {
        g_warning("backend-xcb: Did not find device id: device_type: %d", device_type);
        return XCB_NONE;
    }

    // return
    return device_id;
}

#endif /* USE_XCB */
