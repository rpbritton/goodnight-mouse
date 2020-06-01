import pyatspi

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
