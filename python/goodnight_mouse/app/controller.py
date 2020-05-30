import pyatspi
from Xlib.keysymdef import miscellany as keysym

from .config import Config, WindowConfig
from .overlay import Overlay

from .focus import FocusHandler
from .mouse import MouseHandler
from .keys import KeysHandler
from .action import new_action

class Controller:
    def __init__(self, config: Config, overlay: Overlay):
        self.base_config = config
        self.overlay = overlay

        self.focus_handler = FocusHandler(self.end)
        self.mouse_handler = MouseHandler(self.end)
        self.keys_handler = KeysHandler(self.handle_key)

    def start(self):
        self.focus_handler.start()

        self.window = self.focus_handler.window
        if self.window is None:
            self.stop()
            return

        self.config = WindowConfig(self.base_config, self.window)

        self.mouse_handler.start()
        self.keys_handler.start()
        self.overlay.start(self.config)

        actions = []
        for accessible, action_type in self.config.get_actions():
            actions.append(new_action(self.config, accessible, action_type))
        if len(actions) < 1:
            self.stop()
            return
        print(actions)

        pyatspi.Registry.start()

        self.stop()

    def stop(self):
        self.overlay.stop()
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