import time

import pyatspi
from Xlib import X, display, ext

from .subscription import Subscription
from .utils import ImmediateTimeout


class Mouse(Subscription):
    _MOUSE_EVENTS = ["mouse:button"]

    def __enter__(self):
        super().__enter__()

        ImmediateTimeout.enable()
        pyatspi.Registry.registerEventListener(
            self._handle, *self._MOUSE_EVENTS)
        ImmediateTimeout.disable()

        return self

    def __exit__(self, *args):
        super().__exit__(*args)

        pyatspi.Registry.deregisterEventListener(
            self._handle, *self._MOUSE_EVENTS)

    def _handle(self, event):
        self.notify()

    @classmethod
    def click(cls, x, y):
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
