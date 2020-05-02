import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, Gdk

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
        css_provider = Gtk.CssProvider()
        css_provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.window = Gtk.Window(type=Gtk.WindowType.POPUP)
        self.window.show_all()
        
        for tag in tags.get_tags():
            self._create_tag_gui(tag)

        Gtk.main()

    def _create_tag_gui(self, tag): 
        # TODO: delete old gui
        if tag.code == None:
            return
        
        tag.window = Gtk.Window(type = Gtk.WindowType.POPUP)
        tag.window.move(tag.x, tag.y)
        print(tag.code, tag.x, tag.y)
        tag.window.set_decorated(False)
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