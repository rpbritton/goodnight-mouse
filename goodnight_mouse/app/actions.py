import logging
import re
from typing import List, Set

import pyatspi
import numexpr

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gdk, Gtk

from .config import (
    WindowConfig,
    ActionConfig,
    ExecuteConfig,
    ExecuteActionConfig,
    ExecuteKeyConfig,
    ExecuteMouseConfig,
    ExecuteFocusConfig,
)
from .codes import Codes
from .utils import Emulation
from .overlay import Overlay

# TODO: use re.compile and match


class Actions:
    def __init__(self, overlay: Overlay):
        self._overlay = overlay

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
        self.actions = [Action(config, self._overlay)
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

    def execute(self, code=None):
        if code is None:
            code = self.code

        for action in self.actions:
            if action.code == code:
                action.execute()


class Action:
    def __init__(self, config: ActionConfig, overlay: Overlay):
        self._config = config
        self.accessible = self._config.accessible
        self._overlay = overlay

        self._code = []
        self._active_code = []

        self.widget_width = self.height = 0
        # self.window_x = self.window_y = 0
        # self.window_center_x = self.window_center_y = 0
        # self.screen_x = self.screen_y = 0
        # self.screen_center_x = self.screen_center_y = 0
        self.x = self.y = 0
        self.center_x = self.center_y = 0

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
        self._overlay.remove(self.box)

    def show(self):
        self.box.show_all()

        component = self.accessible.queryComponent()
        bounding_box = component.getExtents(pyatspi.component.XY_SCREEN)
        self.x, self.y = bounding_box.x, bounding_box.y
        self.widget_width, self.widget_height = bounding_box.width, bounding_box.height

        popup_size = self.box.get_preferred_size()
        self.popup_width, self.popup_height = popup_size[1].width, popup_size[1].height

        self.center_x = self.x + self.widget_width // 2
        self.center_y = self.y + self.widget_height // 2

        measurements = {
            "x": self.x,
            "y": self.y,
            "widget_width": self.widget_width,
            "widget_height": self.widget_height,
            "popup_width": self.popup_width,
            "popup_height": self.popup_height,
        }

        x = numexpr.evaluate(self._config.x.substitute(**measurements)).item()
        y = numexpr.evaluate(self._config.y.substitute(**measurements)).item()

        if self.visible:
            self._overlay.move(self.box, x, y)
        else:
            self._overlay.put(self.box, x, y)

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

    def execute(self):
        executions = {
            "action": self.execute_action,
            "key": self.execute_key,
            "mouse": self.execute_mouse,
            "focus": self.execute_focus,
        }

        for execution in self._config.execute:
            if execution.type in executions:
                executions[execution.type](execution)

    def execute_action(self, config: ExecuteActionConfig):
        logging.debug("executing action...")

        try:
            action = self.accessible.queryAction()
            for index in range(action.get_nActions()):
                if re.search(config.match, action.getName(index)) is not None:
                    action.doAction(index)
                    break
        except NotImplementedError:
            pass

    def execute_key(self, config: ExecuteKeyConfig):
        logging.debug("executing key...")

        Emulation.key(config.key, config.action, 0)

    def execute_mouse(self, config: ExecuteMouseConfig):
        logging.debug("executing mouse...")

        Emulation.mouse(config.button, config.action,
                        self.center_x, self.center_y)

    def execute_focus(self, config: ExecuteFocusConfig):
        logging.debug("executing focus...")

        component = self.accessible.queryComponent()
        component.grabFocus()
