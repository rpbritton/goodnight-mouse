#include <stdio.h>

#include <glib-object.h>
#include <atspi/atspi-registry.h>

#include "focus.h"

static unsigned long get_property(Display *display, Window window, char *property_name) {
    Atom property, actual_property;
    int actual_format;
    unsigned long n_items, n_remaining_items;
    unsigned char *data;

    property = XInternAtom(display, property_name, False);
    if (XGetWindowProperty(display, window, property,
                           0, 1, False, AnyPropertyType,
                           &actual_property, &actual_format,
                           &n_items, &n_remaining_items, &data) != Success || data == NULL) {
        printf("could not get property %s\n", property_name);
        exit(1);
    }

    if (!n_items) {
        printf("could not get value for window property %s\n", property_name);
        exit(1);
    }

    unsigned long result = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
    XFree(data);
    return result;
}

AtspiAccessible *focus_get_active_window(Display *display) {
    Window root = DefaultRootWindow(display);
    if (!root) {
        printf("could not get root display\n");
        exit(1);
    }
    Window active_window = get_property(display, root, "_NET_ACTIVE_WINDOW");
    if (!active_window) {
        printf("could not get active window\n");
        exit(1);
    }
    unsigned long active_pid = get_property(display, active_window, "_NET_WM_PID");

    GError *error;
    AtspiAccessible *desktop = atspi_get_desktop(0);
    if (!desktop) {
        printf("could not get desktop\n");
        exit(1);
    }

    int application_count = atspi_accessible_get_child_count(desktop, &error);
    if (!error) {
        printf("could not get desktop child count: %s\n", error->message);
        exit(1);
    }

    AtspiAccessible *application;
    for (int application_index = 0; application_index < application_count; application_index++) {
        application = atspi_accessible_get_child_at_index(desktop, application_index, &error);
        if (!error) {
            printf("could not get application: %s\n", error->message);
            exit(1);
        }

        int pid = atspi_accessible_get_process_id(application, &error);
        if (!error) {
            printf("could not get process id of application: %s\n", error->message);
            exit(1);
        }

        if (active_pid == pid) {
            break;
        }
    }

    return application;
}