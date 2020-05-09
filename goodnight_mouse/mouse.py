import gi
gi.require_version("Atspi", "2.0")
from gi.repository import Atspi

class MouseHandler:
    def __init__(self):
        self._listener = Atspi.DeviceListener.new(self._handle)
        # TODO: check return value
        Atspi.register_device_event_listener(self._listener, 0, None)

    def _handle(self, event):
        exit()