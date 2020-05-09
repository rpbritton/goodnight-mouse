import gi
gi.require_version("Atspi", "2.0")
from gi.repository import Atspi

_valid_window_states = [Atspi.StateType.ACTIVE, Atspi.StateType.VISIBLE, Atspi.StateType.SHOWING]

class FocusHandler:
    def __init__(self):
        self._listener = Atspi.EventListener.new(self._handle)
        # TODO: return value?
        self._listener.register("window:deactivate")


    def _handle(self, event):
        # TODO: This may need to get more advanced
        exit()

def get_focused_window():
    # TODO: figure out better way to find focused window
    desktop = Atspi.get_desktop(0)

    active_windows = []
    for application_index in range(desktop.get_child_count()):
        application = desktop.get_child_at_index(application_index)
        for window_index in range(application.get_child_count()):
            window = application.get_child_at_index(window_index)
            states = window.get_state_set()
            for state in _valid_window_states:
                if not states.contains(state):
                    print(application.name)
                    break
            else:
                print(application.name, "---------- focused")
                active_windows.append(window)
    if len(active_windows) != 1:
        # TODO: raise exception
        exit()

    return active_windows[0]