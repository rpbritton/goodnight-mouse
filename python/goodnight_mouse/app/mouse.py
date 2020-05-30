import pyatspi
from Xlib import X, display, ext
import time

MOUSE_EVENTS = ["mouse:button"]

class MouseHandler:
    def __init__(self, callback):
        self.callback = callback

    def start(self):
        pyatspi.Registry.registerEventListener(self.handle, *MOUSE_EVENTS)

    def stop(self):
        pyatspi.Registry.deregisterEventListener(self.handle, *MOUSE_EVENTS)

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