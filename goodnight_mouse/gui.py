import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk, GLib

css = b"""
.tag {
    background-color: red;
    margin: 0;
    padding: 0;
}

.code_incomplete {
    color: blue;
}

.code_complete {
    color: green;
}
"""

class Gui:
    def __init__(self, tags):
        self.events_window = None

        css_provider = Gtk.CssProvider()
        css_provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        for tag in tags:
            self._create_tag_gui(tag)

        tags[2].window.hide()

        self._create_events_window()

    def main(self):
        # TODO: check return value
        self.seat = Gdk.Display.get_default().get_default_seat()
        # TODO: replace with ALL
        self.seat.grab(self.events_window.get_window(), Gdk.SeatCapabilities.POINTER, True, None, None, None, None)

        Gtk.main()

    def _create_events_window(self):
        # # TODO
        # if self.events_window != None:
        #     return

        self.events_window = Gtk.Invisible()
        self.events_window.connect("button-press-event", self._events_window_callback)
        self.events_window.show_all()

    def _events_window_callback(self, window, event):
        print(window, event)

    def _create_tag_gui(self, tag): 
        # TODO: delete old gui
        if tag.code == None:
            return
        
        tag.window = Gtk.Window(type=Gtk.WindowType.POPUP)
        tag.window.move(tag.x, tag.y)
        # tag.window.set_decorated(False) # TODO: not needed?
        # TODO: make window transparent

        button = Gtk.Button()
        button.get_style_context().add_class("tag")

        labels = Gtk.Box()
        labels.set_halign(Gtk.Align.CENTER)
        for code_char in tag.code:
            code_char_label = Gtk.Label(code_char)
            code_char_label.get_style_context().add_class("code_incomplete")
            labels.add(code_char_label)

        button.add(labels)

        tag.window.add(button)
        tag.window.show_all()