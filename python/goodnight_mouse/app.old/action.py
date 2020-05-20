import pyatspi

from .popup import Popup
from .mouse import click

class Action:
    def __init__(self, code, accessible):
        self.code = code
        self.accessible = accessible

        # TODO: check return
        component = accessible.queryComponent()
        self.x, self.y = component.getPosition(pyatspi.component.XY_SCREEN)
        self.w, self.h = component.getSize()
        self.center_x, self.center_y = self.x + self.w // 2, self.y + self.h // 2

        self._popup = Popup(self.x, self.y, self.code)

    def valid(self, code):
        return self.code.startswith(code)

    def apply(self, code):
        if self.valid(code):
            self._popup.show(len(code))
        else:
            self._popup.hide()

    def match(self, code):
        return self.code == code

    def do(self):
        None

class ActionClick(Action):
    def do(self):
        click(self.center_x, self.center_y)

class ActionFocus(Action):
    def do(self):
        self.accessible.grab_focus()

class ActionPress(Action):
    def do(self):
        None
class ActionNative(Action):
    def do(self):
        actions = self.accessible.queryAction()
        if actions.get_nActions() > 0:
            actions.doAction(0)

ACTION_MAP = {
    "click": ActionClick,
    "focus": ActionFocus,
    "press": ActionPress,
    "native": ActionNative,
}