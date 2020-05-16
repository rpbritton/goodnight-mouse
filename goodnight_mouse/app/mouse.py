import pyatspi
from Xlib import X, display, ext
import time

class MouseHandler:
    def __init__(self):
        pyatspi.Registry().registerEventListener(self.handle, "mouse:button")

    def handle(self, event):
        exit()

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