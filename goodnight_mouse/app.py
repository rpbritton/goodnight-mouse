import pyatspi

from .tags import create_tags
from .gui import Gui

window_states = [pyatspi.STATE_ACTIVE, pyatspi.STATE_VISIBLE, pyatspi.STATE_SHOWING]

def run():
    desktop = pyatspi.Registry().getDesktop(0)

    active_window = []
    for application in desktop:
        print(application.name)
        for window in application:
            states = window.getState()
            for state in window_states:
                if not states.contains(state):
                    break
            else:
                print("focused")
                active_window.append(window)

    if len(active_window) != 1:
        return

    tags = create_tags(window)
    gui = Gui(tags)

    gui.main()

    # event_window = Gtk.Window()
    # event_window.connect("key_press_event", lambda x: print(x))
    # event_window.connect("destroy", Gtk.main_quit)
    # Gdk.keyboard_grab(event_window, True, 0)
