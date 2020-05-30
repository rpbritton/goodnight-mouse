import pyatspi
from Xlib.keysymdef import miscellany as keysym
import time

from .controller import Controller
from .config import Config, WindowConfig
from .registry import BackgroundController
from .overlay import OverlayController

from .focus import FocusController
from .mouse import MouseController
from .keys import KeysController
from .action import ActionsController

class ForegroundController(Controller):
    def __init__(self, config: Config, background_controller: BackgroundController, overlay_controller: OverlayController):
        super().__init__()

        self.base_config = config

        self.background_controller = background_controller
        self.overlay_controller = overlay_controller
        self.focus_controller = FocusController(self.stop)
        self.mouse_controller = MouseController(self.stop)
        self.keys_controller = KeysController(self.handle_key)
        self.actions_controller = ActionsController()

    def start(self, window: pyatspi.Accessible):
        if not super().start(): return

        config = WindowConfig(self.base_config, window)

        self.focus_controller.start()
        self.mouse_controller.start()
        self.keys_controller.start()
        self.overlay_controller.start(config)
        self.actions_controller.start(config, self.overlay_controller.container)

    def stop(self):
        if not super().stop(): return

        self.actions_controller.stop()
        self.overlay_controller.stop()
        self.keys_controller.stop()
        self.mouse_controller.stop()
        self.focus_controller.stop()

    def handle_key(self, key):
        if key == keysym.XK_Escape:
            print("escape")
            self.stop()
        elif key == keysym.XK_BackSpace:
            print("backspace")
        elif 0x00 <= key <= 0xFF:
            print(chr(key))