import logging

import cairo
import pyatspi

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gdk, Gtk

from .config import WindowConfig


class Overlay:
    def __init__(self):
        self._config = WindowConfig()

        self.window = Gtk.Window(type=Gtk.WindowType.POPUP)
        self.window.get_style_context().add_class("overlay_window")
        self.window.set_title("goodnight_mouse")
        self.window.set_visual(self.window.get_screen().get_rgba_visual())
        self.window.set_accept_focus(False)
        self.window.set_sensitive(False)
        # self.window.set_type_hint(Gdk.WindowTypeHint.POPUP_MENU)

        self.container = Gtk.Fixed()
        self.container.get_style_context().add_class("action_container")
        self.window.add(self.container)

        def remove_input(*args):
            self.window.input_shape_combine_region(
                cairo.Region(cairo.RectangleInt()))
        self.window.connect("draw", remove_input)

        self._css_provider = None
        self.visible = False

    def __call__(self, config: WindowConfig):
        self._config = config

        return self

    def __enter__(self):
        self._css_provider = self._config.css
        self.window.get_style_context().add_provider(
            self._css_provider, Gtk.STYLE_PROVIDER_PRIORITY_SETTINGS)

        self.clear()
        self.show()

        return self

    def __exit__(self, *args):
        self.hide()
        self.clear()

        if self._css_provider is not None:
            self.window.get_style_context().remove_provider(self._css_provider)
            self._css_provider = None

    def show(self):
        if self._config.accessible is None:
            self.hide()
            return
        if self.visible is True:
            return

        logging.debug("showing overlay window...")

        component = self._config.accessible.queryComponent()
        bounding_box = component.getExtents(pyatspi.component.XY_SCREEN)
        self.window.move(bounding_box.x, bounding_box.y)
        self.window.resize(bounding_box.width, bounding_box.height)

        self.window.show_all()
        self.visible = True

    def hide(self):
        if self.visible is not True:
            return

        logging.debug("hiding overlay window...")

        self.window.hide()
        self.visible = False

    def clear(self):
        for child in self.container.get_children():
            self.container.remove(child)
