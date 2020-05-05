import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
gi.require_version("Atspi", "2.0")
from gi.repository import Gtk, Gdk, Atspi

from .css import css
from .tags import create_tags
from .input import InputHandler

valid_window_states = [Atspi.StateType.ACTIVE, Atspi.StateType.VISIBLE, Atspi.StateType.SHOWING]

def run():
    desktop = Atspi.get_desktop(0)

    active_windows = []
    for application_index in range(desktop.get_child_count()):
        application = desktop.get_child_at_index(application_index)
        for window_index in range(application.get_child_count()):
            window = application.get_child_at_index(window_index)
            states = window.get_state_set()
            for state in valid_window_states:
                if not states.contains(state):
                    print(application.name)
                    break
            else:
                print(application.name, "---------- focused")
                active_windows.append(window)

    if len(active_windows) != 1:
        exit()
    
    window = active_windows[0]

    css_provider = Gtk.CssProvider()
    css_provider.load_from_data(css)
    Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

    tags = create_tags(window)

    input_handler = InputHandler(tags)

    Gtk.main()
