import pyatspi

from .popup import Popup
from .mouse import click

class Action:
    ROLES_FOCUS = set([
        pyatspi.ROLE_DATE_EDITOR,
        pyatspi.ROLE_TEXT,
        pyatspi.ROLE_ENTRY,
        pyatspi.ROLE_PASSWORD_TEXT,
        pyatspi.ROLE_SLIDER,
        pyatspi.ROLE_DIAL])

    ROLES_CLICK = set([
        pyatspi.ROLE_PUSH_BUTTON,
        pyatspi.ROLE_TOGGLE_BUTTON,
        pyatspi.ROLE_COMBO_BOX,
        # pyatspi.ROLE_LIST_ITEM,
        pyatspi.ROLE_MENU,
        pyatspi.ROLE_MENU_ITEM,
        pyatspi.ROLE_PAGE_TAB,
        pyatspi.ROLE_RADIO_BUTTON,
        pyatspi.ROLE_SPIN_BUTTON,
        pyatspi.ROLE_CHECK_BOX,
        pyatspi.ROLE_CHECK_MENU_ITEM,
        pyatspi.ROLE_LINK,
    ])

    ROLES = ROLES_FOCUS | ROLES_CLICK

    def __init__(self, code, accessible):
        self.code = code
        self.accessible = accessible

        self.role = accessible.getRole()

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
        # if self.role in Action.ROLES_CLICK:
        #     try:
        #         actions = self.accessible.queryAction()
        #     except NotImplementedError:
        #         click(self.center_x, self.center_y)
        #     else:
        #         if actions.get_nActions() > 0:
        #             actions.doAction(0)
        #         else:
        #             click(self.center_x, self.center_y)
        # elif self.role in Action.ROLES_FOCUS:
        #     self.accessible.grab_focus()

        # TODO: make control key held at certain times
        # it should work through x lib
        # Check if already sent methinks, make sure to not release it if already pressed
