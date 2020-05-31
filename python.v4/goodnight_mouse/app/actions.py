import pyatspi

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gdk, Gtk

from .config import WindowConfig


class Actions:
    def __init__(self, config: WindowConfig, container: Gtk.Fixed):
        self._actions = []
        self._config = config
        self._container = container

    def __enter__(self):
        self._actions = []
        for action_type, accessible in self._config.get_actions():
            action = Action.new(self._config, action_type,
                                accessible, self._container)
            self._actions.append(action)

        return self

    def __exit__(self, *args):
        for action in self._actions:
            action.clear()

    def n_valid_actions(self):
        return sum(1 for action in self._actions if action.valid())

    # interpret keys


class Action:
    @staticmethod
    def new(config, action_type: str, *args):
        if action_type in _ACTION_TYPES:
            return _ACTION_TYPES[action_type](config, *args)
        return None

    def __init__(self, config: WindowConfig, accessible: pyatspi.Accessible, container: Gtk.Fixed):
        self._config = config
        self._accessible = accessible
        self._container = container

        # TODO: make into function
        component = self._accessible.queryComponent()
        self._x, self._y = component.getPosition(pyatspi.component.XY_SCREEN)
        self._w, self._h = component.getSize()
        self._center_x, self._center_y = self._x + self._w // 2, self._y + self._h // 2

        self.popup = Gtk.Box()
        self.popup.get_style_context().add_class("action_box")
        self.popup.set_halign(Gtk.Align.CENTER)

        # TODO: add code to parameters and have a set_code function

        # self._container.add

    def valid(self):
        return True

    def clear(self):
        self._container.remove(self.popup)

    def do(self):
        pass


class _NativeAction(Action):
    def do(self):
        pass


class _PressAction(Action):
    def do(self):
        pass


class _ClickAction(Action):
    def do(self):
        pass


class _FocusAction(Action):
    def do(self):
        pass


_ACTION_TYPES = {
    "native": _NativeAction,
    "press": _PressAction,
    "click": _ClickAction,
    "focus": _FocusAction,
}
