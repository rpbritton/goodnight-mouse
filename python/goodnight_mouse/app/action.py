import pyatspi
import cairo

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

def new_action(config, accessible, action_type):
    if action_type in ACTION_TYPES:
        return ACTION_TYPES[action_type](config, accessible)
    return None

class Action:
    def __init__(self, config, accessible):
        self.config = config
        self.accessible = accessible

        # TODO: check return
        component = accessible.queryComponent()
        self.x, self.y = component.getPosition(pyatspi.component.XY_SCREEN)
        self.w, self.h = component.getSize()
        self.center_x, self.center_y = self.x + self.w // 2, self.y + self.h // 2

        self.window = Gtk.Window(type=Gtk.WindowType.POPUP)
        self.window.get_style_context().add_provider(self.config.css, Gtk.STYLE_PROVIDER_PRIORITY_SETTINGS)
        self.window.get_style_context().add_class("action_window")

        self.window.set_title("goodnight_mouse:" + str(self.accessible.id))
        self.window.set_visual(self.window.get_screen().get_rgba_visual())
        # TODO: adjustable alignment (center could be nice)
        self.window.move(self.x, self.y)
        self.window.resize(20, 20)

        def remove_input(widget, cairo_context):
            self.window.input_shape_combine_region(cairo.Region(cairo.RectangleInt()))
        self.window.connect("draw", remove_input)

        self.box = Gtk.Box()
        self.box.get_style_context().add_class("action_box")
        self.box.set_halign(Gtk.Align.CENTER)
        self.window.add(self.box)

    def do(self):
        None

class NativeAction(Action):
    def do(self):
        None

class PressAction(Action):
    def do(self):
        None

class ClickAction(Action):
    def do(self):
        None

class FocusAction(Action):
    def do(self):
        None

ACTION_TYPES = {
    "native": NativeAction,
    "press": PressAction,
    "click": ClickAction,
    "focus": FocusAction,
}