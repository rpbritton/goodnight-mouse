from Xlib.display import Display
from Xlib import Xatom
# from ewmh import EWMH
import pyatspi

def get_focused_window():
    # ewmh = EWMH()
    # active_window = ewmh.getActiveWindow()
    # if active_window is None:
    #     return None
    # active_pid = ewmh.getWmPid(active_window)

    display = Display()
    root_window = display.screen().root

    active_window_atom = display.get_atom("_NET_ACTIVE_WINDOW")
    active_window_property = root_window.get_property(active_window_atom, Xatom.WINDOW, 0, 1)
    active_window = display.create_resource_object("window", active_window_property.value[0])

    active_pid_atom = display.get_atom("_NET_WM_PID")
    active_pid_property = active_window.get_property(active_pid_atom, Xatom.CARDINAL, 0, 1)
    active_pid = int(active_pid_property.value[0])

    desktop = pyatspi.Registry.getDesktop(0)
    for application in desktop:
        if application.get_process_id() == active_pid:
            for window in application:
                states = window.getState().getStates()
                if pyatspi.STATE_ACTIVE in states:
                    return window

    return None