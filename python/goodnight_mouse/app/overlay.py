import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

from .config import WindowConfig

class Overlay:
    def __init__(self):
        self.window = Gtk.Window(type=Gtk.WindowType.POPUP)
        # self.window.get_style_context().add_provider(self.config.css, Gtk.STYLE_PROVIDER_PRIORITY_SETTINGS)
        # self.window.get_style_context().add_class("overlay_window")

        self.window.set_title("goodnight_mouse")
        self.window.set_visual(self.window.get_screen().get_rgba_visual())
        # # # TODO: adjustable alignment (center could be nice)
        # # self.window.move(self.x, self.y)
        # self.window.resize(20, 20)
        self.window.set_accept_focus(False)
        self.window.set_sensitive(False)
        self.window.set_type_hint(Gdk.WindowTypeHint.POPUP_MENU)

        self.window.show_all()
        # self.window.hide()

    def start(self, config: WindowConfig):
        pass