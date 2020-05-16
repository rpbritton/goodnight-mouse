import pyatspi

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

from .css import css

from .focus import FocusHandler
from .action_list import ActionList
from .keys import KeysHandler
from .mouse import MouseHandler

def main(config):
    css_provider = Gtk.CssProvider()
    css_provider.load_from_data(css)
    Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

    # TODO: catch no focus exception?
    focus_handler = FocusHandler()

    action_list = ActionList(focus_handler.get_window())

    keys_handler = KeysHandler(action_list)
    mouse_handler = MouseHandler()

    pyatspi.Registry().start()

    action_list.do()