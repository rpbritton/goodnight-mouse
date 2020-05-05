import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

css = b"""
.tag {
    background-color: red;
    margin: 0;
    padding: 0;
}

.code {
    color: blue;
}

.code_typed {
    color: green;
}
"""

class Gui:
    def __init__(self):
        css_provider = Gtk.CssProvider()
        css_provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)
    
    def create_tags(self, tags):
        for tag in tags:
            self._create_tag(tag)

    def main(self):
        Gtk.main()

    def create_input(self, input_handler):
        # # TODO
        # if self.events_window != None:
        #     return

        input_handler.window = Gtk.Invisible()
        input_handler.window.connect("key-press-event", self.input_handler.key_press)
        input_handler.window.show_all()

        # TODO: check return value
        self.seat = Gdk.Display.get_default().get_default_seat()
        self.seat.grab(input_handler.window.get_window(), Gdk.SeatCapabilities.ALL, True, None, None, None, None)

    def _create_tag(self, tag): 
        # TODO: delete old gui
        if tag.code == None:
            return
        
        tag.window = Gtk.Window(type = Gtk.WindowType.POPUP)
        tag.window.move(tag.x, tag.y)
        # tag.window.set_decorated(False) # TODO: not needed?
        # TODO: make window transparent

        button = Gtk.Button()
        button.get_style_context().add_class("tag")

        labels = Gtk.Box()
        labels.set_halign(Gtk.Align.CENTER)
        for code_char in tag.code:
            code_char_label = Gtk.Label(code_char)
            code_char_label.get_style_context().add_class("code")
            labels.add(code_char_label)

        button.add(labels)

        tag.window.add(button)
        tag.window.show_all()