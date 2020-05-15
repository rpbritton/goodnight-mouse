import cairo

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

class Popup:
    def __init__(self, x, y, code):
        self._window = Gtk.Window(type = Gtk.WindowType.POPUP)

        # TODO: adjustable alignment (center could be nice)
        self._window.move(x, y)

        # self.window.set_decorated(False) # TODO: not needed for popup windows?
        # TODO: make window semi-transparent?

        labels = Gtk.Box()
        labels.set_halign(Gtk.Align.CENTER)
        labels.get_style_context().add_class("popup")

        self._key_labels = []
        for key in iter(code):
            key_label = Gtk.Label(key)
            key_label.get_style_context().add_class("popup_key")
            self._key_labels.append(key_label)
            labels.add(key_label)
        self._window.add(labels)

        self._window.show_all()

        # turns off pointer events
        self._window.input_shape_combine_region(cairo.Region(cairo.RectangleInt()))

    def show(self, num_satisified):
        for index in range(len(self._key_labels)):
            style_context = self._key_labels[index].get_style_context()
            if index < num_satisified:
                style_context.add_class("popup_key_satisfied")
            else:
                style_context.remove_class("popup_key_satisfied")
        self._window.show()

    def hide(self):
        self._window.hide()