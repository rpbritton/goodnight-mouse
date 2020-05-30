import pyatspi

from .controller import Controller
from .focus import get_focused_window

# TODO: are these always sent
ACTIVATE_EVENTS = ["window:activate"]
DEACTIVATE_EVENTS = ["window:deactivate"]

class RegistryController(Controller):
    def __init__(self):
        super().__init__()

        self.active_window = None

    def start(self):
        if not super().start(): return

        pyatspi.Registry.registerEventListener(self.activate_handle, *ACTIVATE_EVENTS)
        pyatspi.Registry.registerEventListener(self.deactivate_handle, *DEACTIVATE_EVENTS)

        self.active_window = get_focused_window()

    def stop(self):
        if not super().stop(): return

        pyatspi.Registry.registerEventListener(self.activate_handle, *ACTIVATE_EVENTS)
        pyatspi.Registry.registerEventListener(self.deactivate_handle, *DEACTIVATE_EVENTS)

    def activate_handle(self, event):
        print(event) # TODO
    def activate(self, window: pyatspi.Accessible):
        self.active_window = window

        # this seems like a good function for forcing a cache fetch
        application = window.getApplication()
        application.accessibleId

    def deactivate_handle(self, event):
        print(event) # TODO
    def deactivate(self, window: pyatspi.Accessible):
        # TODO: does this actually work
        if self.active_window is window:
            self.active_window = None

    def get_focused_window(self):
        if self.is_running():
            return self.active_window
        else:
            return get_focused_window()