from Xlib import Xatom, display
import pyatspi

FOCUS_EVENTS = ["window:activate", "window:deactivate"]

class FocusHandler:
    def __init__(self, callback):
        self.callback = callback
        self.started = False

    def start(self):
        if self.started:
            return
        self.started = True

        pyatspi.Registry.registerEventListener(self.handle, *FOCUS_EVENTS)

        self.window = get_focused_window()

    def stop(self):
        if not self.started:
            return
        self.started = False

        pyatspi.Registry.deregisterEventListener(self.handle, *FOCUS_EVENTS)

    def handle(self, event):
        self.callback()

def get_focused_window():
    dis = display.Display()
    root_window = dis.screen().root

    active_window_atom = dis.get_atom("_NET_ACTIVE_WINDOW")
    active_window_property = root_window.get_property(active_window_atom, Xatom.WINDOW, 0, 1)
    active_window = dis.create_resource_object("window", active_window_property.value[0])

    active_pid_atom = dis.get_atom("_NET_WM_PID")
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