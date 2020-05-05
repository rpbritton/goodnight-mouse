import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

import pyatspi

from .css import css
from .tags import create_tags
from .input import InputHandler

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

    css_provider = Gtk.CssProvider()
    css_provider.load_from_data(css)
    Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

    tags = create_tags(window)

    input_handler = InputHandler(tags)

    Gtk.main()
