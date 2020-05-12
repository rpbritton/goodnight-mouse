import pyatspi

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

from .css import css

from .focus import FocusHandler
from .actions import Actions
# from .events import EventsHandler


def start():
    css_provider = Gtk.CssProvider()
    css_provider.load_from_data(css)
    Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

    # TODO: catch no focus exception?
    focus_handler = FocusHandler()

    actions = Actions(focus_handler.get_window())

    # keys_handler = KeysHandler(actions)
    # mouse_handler = MouseHandler()

    # pyatspi.Registry.start()   

    # actions.finish()