import pyatspi
from Xlib.keysymdef import miscellany as keysym

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gdk, Gtk

from .config import WindowConfig
from .codes import Codes
from .utils import Emulation


class Actions:
    def __init__(self, config: WindowConfig, container: Gtk.Fixed):
        self._actions = []
        self._config = config
        self._container = container
        self._code = ""
        self._codes_generator = Codes(self._config.keys)

    def __enter__(self):
        parsed_actions = self._config.get_actions()
        codes = self._codes_generator.generate(len(parsed_actions))

        self._actions = []
        for action_type, accessible, code in zip(*zip(*parsed_actions), codes):
            action = Action.new(self._config, action_type,
                                accessible, code, self._container)
            self._actions.append(action.__enter__())

        return self

    def __exit__(self, *args):
        for action in self._actions:
            action.__exit__(*args)

    def valid_code(self, code):
        return sum(1 for action in self._actions if action.valid_code(code))

    def apply_code(self, code):
        if code == self._code:
            return
        self._code = code

        for action in self._actions:
            action.apply_code(self._code)

    def do(self):
        for action in self._actions:
            if action.matches_code(self._code):
                action.do()


class Action:
    @staticmethod
    def new(config, action_type: str, *args):
        if action_type in _ACTION_TYPES:
            return _ACTION_TYPES[action_type](config, *args)
        return None

    def __init__(self, config: WindowConfig, accessible: pyatspi.Accessible, code: str, container: Gtk.Fixed):
        self._config = config
        self._accessible = accessible
        self._code = code
        self._container = container
        self._x = self._y = self._w = self._h = self._center_x = self._center_y = None
        self._code_box = None
        self._code_labels = []

    def __enter__(self):
        self._refresh_extents()

        self._code_box = Gtk.Box()
        self._code_box.get_style_context().add_provider(
            self._config.css, Gtk.STYLE_PROVIDER_PRIORITY_SETTINGS)
        self._code_box.get_style_context().add_class("action_box")
        self._code_box.set_halign(Gtk.Align.CENTER)
        self._container.put(self._code_box, self._x, self._y)

        self.set_code(self._code)

        self._code_box.show_all()

        return self

    def __exit__(self, *args):
        self._code_box.destroy()

    def _refresh_extents(self):
        component = self._accessible.queryComponent()
        self._x, self._y = component.getPosition(pyatspi.component.XY_WINDOW)
        self._w, self._h = component.getSize()
        self._center_x, self._center_y = self._x + self._w // 2, self._y + self._h // 2

    def set_code(self, code):
        for code_label in self._code_labels:
            code_label.destroy()
        self._code_labels = []

        self._code = code
        for key in iter(self._code):
            key_label = Gtk.Label(key)
            key_label.get_style_context().add_provider(
                self._config.css, Gtk.STYLE_PROVIDER_PRIORITY_SETTINGS)
            key_label.get_style_context().add_class("action_character")

            self._code_labels.append(key_label)
            self._code_box.add(key_label)

    def valid_code(self, code):
        return self._code.startswith(code)

    def matches_code(self, code):
        return self._code == code

    def apply_code(self, code):
        if self.valid_code(code):
            for index in range(0, len(code)):
                self._code_labels[index].get_style_context(
                ).add_class("action_character_active")
            for index in range(len(code), len(self._code)):
                self._code_labels[index].get_style_context(
                ).remove_class("action_character_active")
            self._code_box.show()
        else:
            self._code_box.hide()

    def do(self):
        pass


class _NativeAction(Action):
    def do(self):
        print("doing native")
        action = self._accessible.queryAction()
        action.doAction(0)


class _PressAction(Action):
    def do(self):
        print("doing press")
        component = self._accessible.queryComponent()
        if not component.grabFocus():
            return
        Emulation.key_tap(keysym.XK_Return)


class _ClickAction(Action):
    def do(self):
        print("doing click")
        Emulation.mouse_tap(1, self._center_x, self._center_y)


class _FocusAction(Action):
    def do(self):
        print("doing focus")
        component = self._accessible.queryComponent()
        component.grabFocus()


_ACTION_TYPES = {
    "native": _NativeAction,
    "press": _PressAction,
    "click": _ClickAction,
    "focus": _FocusAction,
}
