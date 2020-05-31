import pyatspi
from Xlib.keysymdef import miscellany as keysym
import time

from .config import Config, WindowConfig

from .focus import Focus
from .mouse import Mouse
from .keys import Keys
from .overlay import Overlay
from .actions import Actions

class Foreground:
    def __init__(self, config: Config, focus: Focus, mouse: Mouse, keys: Keys, overlay: Overlay):
        self._base_config = config
        self._focus = focus
        self._mouse = mouse
        self._keys = keys
        self._overlay = overlay
        self._actions = None

    def __enter__(self):
        self._focus.subscribe(self._handle_focus)
        self._mouse.subscribe(self._handle_mouse)
        self._keys.subscribe(self._handle_keys)

        active_window = self._focus.get_active_window()
        if active_window == None:
            return None
        config = WindowConfig(self._base_config, self._focus.get_active_window())

        self._overlay.begin(config)

        self._actions = Actions(config, self._overlay.get_container())
        print(self._actions.n_valid_actions())

        return self

    def __exit__(self, type, value, traceback):
        self._overlay.finish()
        self._keys.unsubscribe(self._handle_keys)
        self._mouse.unsubscribe(self._handle_mouse)
        self._focus.unsubscribe(self._handle_focus)

    def _handle_keys(self, key):
        if key == keysym.XK_Escape:
            print("escape")
            print("quitting via keys")
            self.quit_loop()
        elif key == keysym.XK_BackSpace:
            print("backspace")
        elif 0x00 <= key <= 0xFF:
            print(chr(key))

    def _handle_mouse(self):
        print("quitting via mouse")
        self.quit_loop()

    def _handle_focus(self, new_active_window: pyatspi.Accessible):
        print("quitting via focus")
        self.quit_loop()

    def _handle_actions(self):
        "quitting via actions"
        self.quit_loop()

    def quit_loop(self):
        pyatspi.Registry.stop()