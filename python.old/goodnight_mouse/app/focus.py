import pyatspi

import gi
gi.require_version("Wnck", "3.0")
from gi.repository import Wnck

import time

_valid_window_states = [pyatspi.STATE_ACTIVE, pyatspi.STATE_VISIBLE, pyatspi.STATE_SHOWING]

class FocusHandler:
    def __init__(self):
        self.window = focused_window()
        # TODO: do something bad if no focused window

        # pyatspi.Registry.registerEventListener(self.handle, "window:deactivate")

    def handle(self, event):
        # TODO: This may need to get more advanced
        exit()

    def get_window(self):
        return self.window

def focused_window():
    times = []
    times.append(time.time())
    # TODO: check if any of these fail (return values)
    screen = Wnck.Screen.get_default()
    screen.force_update()
    active_window = screen.get_active_window()
    active_application = active_window.get_application()
    active_application_pid = active_application.get_pid()
    # print(active_application_pid)
    times.append(time.time())
    for index in range(len(times) - 1):
        print("focused_window: index", index, "time", times[index+1] - times[index])

    desktop = pyatspi.Registry.getDesktop(0)
    # times.append(time.time())
    for application in desktop:
        if application.get_process_id() == active_application_pid:
            return application
            for window in application:
                # times.append(time.time())
                # TODO: check return
                state_set = window.getState()
                for state in _valid_window_states:
                    if not state_set.contains(state):
                        break
                else:
                    # times.append(time.time())
                    return window

    # for index in range(len(times) - 1):
    #     print("focused_window: index", index, "time", times[index+1] - times[index])

    return None