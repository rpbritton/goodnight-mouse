import pyatspi

from .controller import Controller
from .utils import ImmediateTimeout

class KeysController(Controller):
    def __init__(self, callback):
        super().__init__()

        self.callback = callback

    def start(self):
        if not super().start(): return

        # register keystrokes with every possible comination of modifiers
        ImmediateTimeout.enable()
        pyatspi.Registry.registerKeystrokeListener(
            self.handle,
            mask=pyatspi.allModifiers(),
            synchronous=False)
        ImmediateTimeout.disable()

    def stop(self):
        if not super().stop(): return

        # deregister all keystroke listeners
        ImmediateTimeout.enable()
        pyatspi.Registry.deregisterKeystrokeListener(
            self.handle,
            mask=pyatspi.allModifiers())
        ImmediateTimeout.disable()

    def handle(self, event):
        if event.type == pyatspi.deviceevent.KEY_PRESSED_EVENT:
            self.callback(event.id)
        return True