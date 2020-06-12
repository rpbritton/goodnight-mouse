import logging

import pyatspi

from .subscription import Subscription
from .utils import ImmediateTimeout


class KeysEvent:
    def __init__(self, event: pyatspi.deviceevent.DeviceEvent):
        self.event = event
        self.key = event.id
        self.pressed = event.type == pyatspi.KEY_PRESSED_EVENT
        self.modifiers = event.modifiers


class Keys(Subscription):
    def __init__(self):
        super().__init__()

    def __enter__(self):
        logging.debug("registering keys listener")

        ImmediateTimeout.enable()
        pyatspi.Registry.registerKeystrokeListener(
            self._handle,
            mask=pyatspi.allModifiers(),
            synchronous=False)
        ImmediateTimeout.disable()

        return self

    def __exit__(self, *args):
        logging.debug("deregistering keys listener")

        ImmediateTimeout.enable()
        pyatspi.Registry.deregisterKeystrokeListener(
            self._handle,
            mask=pyatspi.allModifiers())
        ImmediateTimeout.disable()

    def _handle(self, event):
        return self.notify(KeysEvent(event))
