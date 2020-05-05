import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

class InputHandler:
    def __init__(self, tags):
        self.tags = tags

    def key_press(self, window, event):
        if event.keyval == Gdk.KEY_Escape:
            exit()
        elif event.keyval == Gdk.KEY_BackSpace:
            print("goback")
        elif event.keyval >= 0 and event.keyval <= 255:
            print(chr(event.keyval))
