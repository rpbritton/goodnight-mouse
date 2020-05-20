#ifndef _FOCUS_H
#define _FOCUS_H

#include <X11/Xlib.h>

#include <glib-object.h>
#include <atspi/atspi-accessible.h>

AtspiAccessible *focus_get_active_window(Display *display);

#endif