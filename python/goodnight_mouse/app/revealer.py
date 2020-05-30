import pyatspi
from Xlib.keysymdef import miscellany as keysym
import time

from .controller import Controller
from .config import Config, WindowConfig
from .overlay import OverlayController

from .focus import FocusController
from .mouse import MouseController
from .keys import KeysController
from .action import ActionsController

class RevealerController(Controller):
    def __init__(self, config: Config, window: pyatspi.Accessible, overlay_controller: OverlayController):
        super().__init__()

        self.config = WindowConfig(config, window)
        self.window = window

        self.overlay_controller = overlay_controller
        self.focus_controller = FocusController(self.end)
        self.mouse_controller = MouseController(self.end)
        self.keys_controller = KeysController(self.handle_key)
        self.actions_controller = ActionsController()

    def start(self):
        times = []
        times.append(time.time())

        if not super().start(): return

        times.append(time.time())
        self.focus_controller.start()
        times.append(time.time())
        self.mouse_controller.start()
        times.append(time.time())
        self.keys_controller.start()
        times.append(time.time())
        self.overlay_controller.start(self.config)
        times.append(time.time())
        self.actions_controller.start(self.config, self.overlay_controller.container)
        times.append(time.time())

        for index in range(len(times) - 1):
            print(index, times[index + 1] - times[index])
        print("final", times[len(times) - 1] - times[0])

        pyatspi.Registry.start()

        self.stop()

    def stop(self):
        if not super().stop(): return

        self.actions_controller.stop()
        self.overlay_controller.stop()
        self.keys_controller.stop()
        self.mouse_controller.stop()
        self.focus_controller.stop()

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