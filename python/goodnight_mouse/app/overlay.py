import pyatspi
import cairo

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

from .config import WindowConfig

class Overlay:
    def __init__(self):
        self.config = None

        self.window = Gtk.Window(type=Gtk.WindowType.POPUP)
        self.style_context = self.window.get_style_context()
        self.style_context.add_class("overlay_window")

        self.window.set_title("goodnight_mouse")
        self.window.set_visual(self.window.get_screen().get_rgba_visual())
        self.window.set_accept_focus(False)
        self.window.set_sensitive(False)
        # self.window.set_type_hint(Gdk.WindowTypeHint.POPUP_MENU)

        def remove_input(widget, cairo_context):
            print("here")
            self.window.input_shape_combine_region(cairo.Region(cairo.RectangleInt()))
        self.window.connect("draw", remove_input)

    def start(self, config: WindowConfig):
        if self.config != None:
            self.stop()

        self.config = config

        self.style_context.add_provider(self.config.css, Gtk.STYLE_PROVIDER_PRIORITY_SETTINGS)

        component = self.config.window.queryComponent()
        self.window.move(*component.getPosition(pyatspi.component.XY_SCREEN))
        self.window.resize(*component.getSize())

        self.window.show_all()

    def stop(self):
        self.style_context.remove_provider(self.config.css)

        self.window.hide()

        self.config = None

        while Gtk.events_pending():
            Gtk.main_iteration()
