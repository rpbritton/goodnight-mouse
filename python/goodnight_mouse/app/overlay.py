import pyatspi
import cairo

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

from .controller import Controller
from .config import WindowConfig
from .action import Action

class OverlayController(Controller):
    def __init__(self):
        super().__init__()

        self.config = None
        self.actions = None

        self.window = Gtk.Window(type=Gtk.WindowType.POPUP)
        self.style_context = self.window.get_style_context()
        self.style_context.add_class("overlay_window")

        self.window.set_title("goodnight_mouse")
        self.window.set_visual(self.window.get_screen().get_rgba_visual())
        self.window.set_accept_focus(False)
        self.window.set_sensitive(False)
        # self.window.set_type_hint(Gdk.WindowTypeHint.POPUP_MENU)

        self.container = Gtk.Fixed()
        self.window.add(self.container)

        def remove_input(widget, cairo_context):
            self.window.input_shape_combine_region(cairo.Region(cairo.RectangleInt()))
        self.window.connect("draw", remove_input)

    def start(self, config: WindowConfig):
        if not super().start(): return

        self.config = config

        self.style_context.add_provider(self.config.css, Gtk.STYLE_PROVIDER_PRIORITY_SETTINGS)

        component = self.config.window.queryComponent()
        bounding_box = component.getExtents(pyatspi.component.XY_SCREEN)
        self.window.move(bounding_box.x, bounding_box.y)
        self.window.resize(bounding_box.width, bounding_box.height)

        self.window.show_all()

    def stop(self):
        if not super().stop(): return

        self.window.hide()
        self.style_context.remove_provider(self.config.css)
        self.config = None
