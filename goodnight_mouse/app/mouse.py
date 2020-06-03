import logging

import pyatspi

from .subscription import Subscription
from .utils import ImmediateTimeout


class Mouse(Subscription):
    _MOUSE_EVENTS = ["mouse:button"]

    def __enter__(self):
        logging.debug("registering mouse listener")

        ImmediateTimeout.enable()
        pyatspi.Registry.registerEventListener(
            self._handle, *self._MOUSE_EVENTS)
        ImmediateTimeout.disable()

        return self

    def __exit__(self, *args):
        logging.debug("deregistering mouse listener")

        pyatspi.Registry.deregisterEventListener(
            self._handle, *self._MOUSE_EVENTS)

    def _handle(self, event):
        self.notify()
