import cairo
import pyatspi

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gdk, Gtk

from .config import WindowConfig


class Overlay:
    def __init__(self):
        super().__init__()

        self._config = None

        self._window = Gtk.Window(type=Gtk.WindowType.POPUP)
        self._window.get_style_context().add_class("overlay_window")
        self._window.set_title("goodnight_mouse")
        self._window.set_visual(self._window.get_screen().get_rgba_visual())
        self._window.set_accept_focus(False)
        self._window.set_sensitive(False)
        # self._window.set_type_hint(Gdk.WindowTypeHint.POPUP_MENU)

        self._container = Gtk.Fixed()
        self._container.get_style_context().add_class("action_container")
        self._window.add(self._container)

        def remove_input(*args):
            self._window.input_shape_combine_region(
                cairo.Region(cairo.RectangleInt()))
        self._window.connect("draw", remove_input)

    def __call__(self, config: WindowConfig):
        self._config = config

        return self

    def __enter__(self):
        self._window.get_style_context().add_provider(
            self._config.css, Gtk.STYLE_PROVIDER_PRIORITY_SETTINGS)

        self.clear()
        self._map_over_window()
        self._window.show_all()

        return self

    def __exit__(self, *args):
        self._window.hide()
        self.clear()

        if self._config is not None:
            self._window.get_style_context().remove_provider(self._config.css)
            self._config = None

    def _map_over_window(self):
        component = self._config.window.queryComponent()
        bounding_box = component.getExtents(pyatspi.component.XY_SCREEN)
        self._window.move(bounding_box.x, bounding_box.y)
        self._window.resize(bounding_box.width, bounding_box.height)

    def get_container(self):
        return self._container

    def clear(self):
        for child in self._container.get_children():
            self._container.remove(child)
