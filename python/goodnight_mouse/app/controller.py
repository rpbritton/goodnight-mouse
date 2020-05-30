import pyatspi
from Xlib.keysymdef import miscellany as keysym

from .config import Config, WindowConfig
from .overlay import Overlay

from .focus import FocusHandler
from .mouse import MouseHandler
from .keys import KeysHandler

class Controller:
    def __init__(self, config: Config, overlay: Overlay):
        self.focus_handler = FocusHandler(self.end)
        self.mouse_handler = MouseHandler(self.end)
        self.keys_handler = KeysHandler(self.handle_key)

    def start(self):
        self.focus_handler.start()
        self.mouse_handler.start()
        self.keys_handler.start()

        # TODO: keep gil? I mean it is single threaded
        pyatspi.Registry.start()

        self.focus_handler.stop()
        self.mouse_handler.stop()
        self.keys_handler.stop()

    def end(self):
        pyatspi.Registry.stop()

    def handle_key(self, key):
        if key == keysym.XK_Escape:
            print("escape")
            self.end()
        elif key == keysym.XK_BackSpace:
            print("backspace")
        elif 0x00 <= key <= 0xFF:
            print(chr(key))