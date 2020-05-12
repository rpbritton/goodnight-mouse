import gi
gi.require_version("Atspi", "2.0")
from gi.repository import Atspi

from .keys import KeysHandler
from .focus import FocusHandler
from .mouse import MouseHandler

class EventsHandler: