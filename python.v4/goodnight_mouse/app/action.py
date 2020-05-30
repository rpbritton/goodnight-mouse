import pyatspi

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

from .config import WindowConfig
from .controller import Controller

class ActionsController(Controller):
    def __init__(self):
        super().__init__()

        self.actions = []
        self.config = None

    def start(self, config: WindowConfig, container: Gtk.Fixed):
        if not super().start(): return

        self.config = config
        self.container = container

        self.actions = []
        for accessible, action_type in self.config.get_actions():
            action = new_action(self.config, accessible, action_type)

            self.actions.append(action)
        if len(self.actions) < 1:
            self.stop()
            return
        print(len(self.actions))
        # self.overlay.set_actions(actions)

    def stop(self):
        if not super().stop(): return

        self.config = None

def new_action(config, accessible: pyatspi.Accessible, action_type: str):
    if action_type in ACTION_TYPES:
        return ACTION_TYPES[action_type](config, accessible)
    return None

class Action:
    def __init__(self, config: WindowConfig, accessible: pyatspi.Accessible):
        self.config = config
        self.accessible = accessible

        component = accessible.queryComponent()
        self.x, self.y = component.getPosition(pyatspi.component.XY_SCREEN)
        self.w, self.h = component.getSize()
        self.center_x, self.center_y = self.x + self.w // 2, self.y + self.h // 2

        self.popup = Gtk.Box()
        self.popup.get_style_context().add_class("action_box")
        self.popup.set_halign(Gtk.Align.CENTER)
        # self.window.add(self.popup)

    def do(self):
        None

class NativeAction(Action):
    def do(self):
        None

class PressAction(Action):
    def do(self):
        None

class ClickAction(Action):
    def do(self):
        None

class FocusAction(Action):
    def do(self):
        None

ACTION_TYPES = {
    "native": NativeAction,
    "press": PressAction,
    "click": ClickAction,
    "focus": FocusAction,
}