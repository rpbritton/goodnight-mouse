import pyatspi

from .subscription import Subscription
from .utils import ImmediateTimeout


class Keys(Subscription):
    def __init__(self):
        super().__init__()

    def __enter__(self):
        super().__enter__()

        ImmediateTimeout.enable()
        pyatspi.Registry.registerKeystrokeListener(
            self._handle,
            mask=pyatspi.allModifiers(),
            synchronous=False)
        ImmediateTimeout.disable()

        return self

    def __exit__(self, *args):
        super().__exit__(*args)

        ImmediateTimeout.enable()
        pyatspi.Registry.deregisterKeystrokeListener(
            self._handle,
            mask=pyatspi.allModifiers())
        ImmediateTimeout.disable()

    def _handle(self, event):
        if event.type == pyatspi.deviceevent.KEY_PRESSED_EVENT:
            self.notify(event.id)
        return True
