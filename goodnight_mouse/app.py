import atexit

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
gi.require_version("Atspi", "2.0")
from gi.repository import Gtk, Gdk, Atspi

from .css import css

from .actions import Actions
from .events import EventsHandler

def run():
    Atspi.init()
    atexit.register(exit_callback)

    css_provider = Gtk.CssProvider()
    css_provider.load_from_data(css)
    Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

    actions = Actions()

    events_handler = EventsHandler(actions)
    events_handler.main()

    actions.finish()

def exit_callback():
    Atspi.exit()