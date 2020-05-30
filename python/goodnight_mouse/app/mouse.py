import pyatspi
from Xlib import X, display, ext
from gi.repository import GLib
import time

from .controller import Controller
from .utils import ImmediateTimeout

MOUSE_EVENTS = ["mouse:button"]

class MouseController(Controller):
    def __init__(self, callback):
        super().__init__()

        self.callback = callback

    def start(self):
        if not super().start(): return

        ImmediateTimeout.enable()
        pyatspi.Registry.registerEventListener(self.handle, *MOUSE_EVENTS)
        ImmediateTimeout.disable()

    def stop(self):
        if not super().stop(): return

        ImmediateTimeout.enable()
        try:
            pyatspi.Registry.deregisterEventListener(self.handle, *MOUSE_EVENTS)
        except GLib.Error as err:
            if err.domain != "atspi_error" or "Did not receive a reply" not in str(err.message):
                raise
        ImmediateTimeout.disable()

    def handle(self, event):
        self.callback()

def click(x, y):
    # TODO: investigate viability of atspi generate_mouse_event
    # (didn't work well from initial testing)

    dis = display.Display()
    root = dis.screen().root
    pointer = root.query_pointer()

    root.warp_pointer(x, y)
    dis.sync()
    time.sleep(0.001)

    ext.xtest.fake_input(dis, X.ButtonPress, 1)
    dis.sync()
    time.sleep(0.001)

    ext.xtest.fake_input(dis, X.ButtonRelease, 1)
    dis.sync()
    time.sleep(0.001)

    root.warp_pointer(pointer.root_x, pointer.root_y)
    dis.sync()