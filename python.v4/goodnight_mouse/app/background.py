import pyatspi

from .controller import Controller
from .focus import get_focused_window

# TODO: are these always sent
ACTIVATE_EVENTS = ["window:activate"]
DEACTIVATE_EVENTS = ["window:deactivate"]

class BackgroundController(Controller):
    def __init__(self):
        super().__init__()

        self.active_window = None

    def start(self):
        if not super().start(): return

        pyatspi.Registry.registerEventListener(self.handle, *ACTIVATE_EVENTS)
        pyatspi.Registry.registerEventListener(self.handle, *DEACTIVATE_EVENTS)

        self.active_window = get_focused_window()

    def stop(self):
        if not super().stop(): return

        pyatspi.Registry.deregisterEventListener(self.handle, *ACTIVATE_EVENTS)
        pyatspi.Registry.deregisterEventListener(self.handle, *DEACTIVATE_EVENTS)

    def handle(self, event):
        if event.type in ACTIVATE_EVENTS:
            self.active_window = event.source
        elif event.type in DEACTIVATE_EVENTS:
            # TODO: does this actually work
            if self.active_window is event.source:
                self.active_window = None

    # def activate_handle(self, event):
    #     print(event)
    #     self.activate(event.source)
    # def activate(self, window: pyatspi.Accessible):
    #     self.active_window = window

    #     # make sure caching/connection occurs
    #     self.active_window.name

    # def deactivate_handle(self, event):
    #     print(event)
    #     self.deactivate(event.source)
    # def deactivate(self, window: pyatspi.Accessible):
    #     # TODO: does this actually work
    #     if self.active_window is window:
    #         self.active_window = None

    def get_window(self):
        if self.is_running():
            return self.active_window
        else:
            return get_focused_window()