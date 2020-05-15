import pyatspi

import gi
gi.require_version("Wnck", "3.0")
from gi.repository import Wnck

_valid_window_states = [pyatspi.STATE_ACTIVE, pyatspi.STATE_VISIBLE, pyatspi.STATE_SHOWING]

class FocusHandler:
    def __init__(self):
        self.window = self._get_focused_window()
        # TODO: do something bad if no focused window

        # TODO: does this return a value
        pyatspi.Registry.registerEventListener(self._focus_lost, "window:deactivate")

    def _focus_lost(self, event):
        # TODO: This may need to get more advanced
        exit()

    def get_window(self):
        return self.window

    def _get_focused_window(self):
        # TODO: check if any of these fail (return values)
        screen = Wnck.Screen.get_default()
        screen.force_update()
        active_window = screen.get_active_window()
        active_application = active_window.get_application()
        active_application_pid = active_application.get_pid()

        desktop = pyatspi.Registry.getDesktop(0)
        for application in desktop:
            if application.get_process_id() == active_application_pid:
                for window in application:
                    # TODO: check return
                    state_set = window.getState()
                    for state in _valid_window_states:
                        if not state_set.contains(state):
                            break
                    else:
                        return window
        
        return None