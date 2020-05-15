from Xlib import X, display, ext
import time

# import gi
# gi.require_version("Atspi", "2.0")
# from gi.repository import Atspi

# class MouseHandler:
#     def __init__(self):
#         self._listener = Atspi.DeviceListener.new(self._handle)
#         # TODO: check return value
#         Atspi.register_device_event_listener(self._listener, 0, None)

#     def _handle(self, event):
#         exit()

def click(x, y):
    # TODO: investigate viability of generate_mouse_event
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