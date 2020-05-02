import pyatspi

from .tags import Tags
from .gui import Gui

window_states = [pyatspi.STATE_ACTIVE, pyatspi.STATE_VISIBLE, pyatspi.STATE_SHOWING]

def run():
    desktop = pyatspi.Registry().getDesktop(0)

    active_window = []
    for application in desktop:
        for window in application:
            states=window.getState()
            for state in window_states:
                if not states.contains(state):
                    break
            else:
                # if application.name == "moserial":
                print(application.name)
                active_window.append(window)

    if len(active_window) != 1:
        return

    tags = Tags(window)
    gui = Gui(tags)

    # event_window = Gtk.Window()
    # event_window.connect("key_press_event", lambda x: print(x))
    # event_window.connect("destroy", Gtk.main_quit)
    # Gdk.keyboard_grab(event_window, True, 0)
