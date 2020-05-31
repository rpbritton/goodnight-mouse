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
                                accessible, "test", self._container)
            self._actions.append(action.__enter__())

        return self

    def __exit__(self, *args):
        for action in self._actions:
            action.__exit__(*args)

    def n_valid_actions(self):
        return sum(1 for action in self._actions if action.valid())

    # interpret keys


class Action:
    @staticmethod
    def new(config, action_type: str, *args):
        if action_type in _ACTION_TYPES:
            return _ACTION_TYPES[action_type](config, *args)
        return None

    def __init__(self, config: WindowConfig, accessible: pyatspi.Accessible, keys: str, container: Gtk.Fixed):
        self._config = config
        self._accessible = accessible
        self._keys = keys
        self._container = container
        self._x = self._y = self._w = self._h = self._center_x = self._center_y = None
        self._keys_box = None
        self._keys_labels = []

    def __enter__(self):
        self._refresh_extents()

        self._keys_box = Gtk.Box()
        self._keys_box.get_style_context().add_class("action_box")
        self._keys_box.set_halign(Gtk.Align.CENTER)

        self.reset_keys(self._keys)

        self._container.put(self._keys_box, self._x, self._y)

        self._keys_box.show_all()

        return self

    def __exit__(self, *args):
        self._container.remove(self._keys_box)

    def _refresh_extents(self):
        component = self._accessible.queryComponent()
        self._x, self._y = component.getPosition(pyatspi.component.XY_SCREEN)
        self._w, self._h = component.getSize()
        self._center_x, self._center_y = self._x + self._w // 2, self._y + self._h // 2

    def reset_keys(self, keys):
        for keys_label in self._keys_labels:
            self._keys_box.remove(keys_label)
        self._keys_labels = []

        self._keys = keys
        for key in iter(self._keys):
            key_label = Gtk.Label(key)
            key_label.get_style_context().add_class("action_key")
            self._keys_box.add(key_label)
            self._keys_labels.append(key_label)

    def valid(self):
        return True

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
