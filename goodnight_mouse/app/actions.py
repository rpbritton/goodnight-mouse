import logging
from typing import List, Set

import pyatspi

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gdk, Gtk

from .config import WindowConfig, ActionConfig
from .codes import Codes
from .utils import Emulation


class Actions:
    def __init__(self, container: Gtk.Fixed):
        self._container = container

        self.actions = []
        self._code = []

        self._config = WindowConfig()
        self._codes_generator = Codes([])
        self._flags = []

    def __call__(self, config: WindowConfig, flags: Set[str] = None):
        self._config = config
        self._codes_generator = Codes(self._config.characters)
        if flags is None:
            flags = set()
        self._flags = flags

        return self

    def __enter__(self):
        self.actions = [Action(config, self._container)
                        for config in self._config.actions(self._flags)]

        codes = self._codes_generator.generate(len(self.actions))
        for action, code in zip(self.actions, codes):
            action.code = code
            action.__enter__()

        self._code = []

        return self

    def __exit__(self, *args):
        for action in self.actions:
            action.__exit__(*args)

    def matches(self, code: List[int]) -> int:
        return sum(1 for action in self.actions if action.valid(code))

    @property
    def code(self):
        return self._code

    @code.setter
    def code(self, code: List[int]):
        if code == self.code:
            return
        self._code = code[:]

        for action in self.actions:
            action.active_code = self.code

    def do(self, code=None):
        if code is None:
            code = self.code

        for action in self.actions:
            if action.code == code:
                action.do()


class Action:
    def __init__(self, config: ActionConfig, container: Gtk.Fixed):
        self._config = config
        self._container = container

        self._code = []
        self._active_code = []

        self.width = self.height = 0
        self.window_x = self.window_y = 0
        self.window_center_x = self.window_center_y = 0
        self.screen_x = self.screen_y = 0
        self.screen_center_x = self.screen_center_y = 0

        self.box = Gtk.Box()
        self.box.get_style_context().add_class("action_box")
        self.box.get_style_context().add_provider(
            self._config.css, Gtk.STYLE_PROVIDER_PRIORITY_SETTINGS)
        self.box.set_halign(Gtk.Align.CENTER)

        self.labels = []
        self.visible = False

    def __enter__(self):
        self.show()

        return self

    def __exit__(self, *args):
        self.hide()
        self._container.remove(self.box)

    def show(self):
        component = self._config.accessible.queryComponent()

        self.width, self.height = component.getSize()

        self.window_x, self.window_y = component.getPosition(
            pyatspi.component.XY_WINDOW)
        self.window_center_x = self.window_x + self.width // 2
        self.window_center_y = self.window_y + self.height // 2

        self.screen_x, self.screen_y = component.getPosition(
            pyatspi.component.XY_SCREEN)
        self.screen_center_x = self.screen_x + self.width // 2
        self.screen_center_y = self.screen_y + self.height // 2

        if self.visible:
            self._container.move(self.box, self.window_x, self.window_y)
        else:
            self._container.put(self.box, self.window_x, self.window_y)

        self.box.show_all()
        self.visible = True

    def hide(self):
        if not self.visible:
            return

        self.box.hide()
        self.visible = False

    @property
    def code(self) -> List[int]:
        return self._code

    @code.setter
    def code(self, code: List[int]):
        while self.labels:
            self.labels.pop().destroy()

        self._code = code
        for keysym in self._code:
            character = chr(Gdk.keyval_to_unicode(keysym))
            label = Gtk.Label(character)
            label.get_style_context().add_provider(
                self._config.css, Gtk.STYLE_PROVIDER_PRIORITY_SETTINGS)
            label.get_style_context().add_class("action_character")

            self.labels.append(label)
            self.box.add(label)

    def valid(self, code: List[int]) -> bool:
        if len(code) > len(self.code):
            return False

        return self.code[:len(code)] == code

    @property
    def active_code(self) -> List[int]:
        return self._active_code

    @active_code.setter
    def active_code(self, code):
        # print("why", code)
        # if code == self.active_code:
        #     return
        self._active_code = code

        if not self.valid(code):
            self.box.hide()
            return

        for index in range(0, len(code)):
            self.labels[index].get_style_context().add_class(
                "action_character_active")
        for index in range(len(code), len(self._code)):
            self.labels[index].get_style_context().remove_class(
                "action_character_active")
        self.box.show()

    def do(self):
        print("doing")


# class _NativeAction(Action):
#     def do(self):
#         logging.debug("doing native action")

#         action = self._accessible.queryAction()
#         action.doAction(0)


# class _PressAction(Action):
#     def do(self):
#         logging.debug("doing press action")

#         component = self._accessible.queryComponent()
#         if not component.grabFocus():
#             return
#         Emulation.key_tap(Gdk.KEY_Return)


# class _ClickAction(Action):
#     def do(self):
#         logging.debug("doing click action")

#         Emulation.mouse_tap(1, self._screen_center_x, self._screen_center_y)


# class _FocusAction(Action):
#     def do(self):
#         logging.debug("doing focus action")

#         component = self._accessible.queryComponent()
#         component.grabFocus()


# _ACTION_TYPES = {
#     "native": _NativeAction,
#     "press": _PressAction,
#     "click": _ClickAction,
#     "focus": _FocusAction,
# }
