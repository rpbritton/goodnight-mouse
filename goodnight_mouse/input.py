import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

class InputHandler:
    def __init__(self, tags):
        self.keys = ""
        self.tags = tags
        self._pressed = False

        self._create_window()

        self._update_tags()

    def _create_window(self):
        # # TODO
        # if self.events_window != None:
        #     return

        self._window = Gtk.Invisible()
        self._window.connect("key-press-event", self._key_press)
        self._window.connect("button-press-event", self._mouse_down)
        self._window.connect("button-release-event", self._mouse_up)
        self._window.show_all()

        seat = Gdk.Display.get_default().get_default_seat()
        status = seat.grab(self._window.get_window(), Gdk.SeatCapabilities.ALL, True, None, None, None, None)
        if status != Gdk.GrabStatus.SUCCESS:
            # TODO: tell the user
            exit()
    
    def _mouse_down(self, window, event):
        if event.button == 1:
            self._pressed = True
    
    def _mouse_up(self, window, event):
        if event.button == 1 and self._pressed:
            exit()
        else:
            self._pressed = False

    def _key_press(self, window, event):
        prev_keys = self.keys

        if event.keyval == Gdk.KEY_Escape:
            exit()
        elif event.keyval == Gdk.KEY_BackSpace:
            self.keys = self.keys[:-1]

            for tag in self.tags:
                tag.update(self.keys)
        elif event.keyval >= 0 and event.keyval <= 255:
            self.keys += chr(event.keyval)

            # reset keys if bad input (no matches)
            for tag in self.tags:
                if tag.keys.startswith(self.keys):
                    break
            else:
                self.keys = ""

        if self.keys != prev_keys:
            self._update_tags()

    def _update_tags(self):
        for tag in self.tags:
            tag.update(self.keys)
