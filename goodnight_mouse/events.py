import gi
gi.require_version("Atspi", "2.0")
from gi.repository import Atspi

from .keys import KeysHandler
from .focus import FocusHandler
from .mouse import MouseHandler

class EventsHandler:
    def __init__(self, actions):
        self._keys_handler = KeysHandler(actions)
        # TODO: make sure focus handler has focused
        self._focus_handler = FocusHandler()
        self._mouse_handler = MouseHandler()
    
    def main(self):
        Atspi.event_main()