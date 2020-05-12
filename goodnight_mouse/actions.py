import random
import time
import pyatspi

from Xlib import X, display, ext

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
gi.require_version("Atspi", "2.0")
from gi.repository import Gtk, Gdk

from .popup import Popup
from .codes import Codes

_code_chars = "fjdkghsla"

class Actions:
    def __init__(self, window):
        self.current_code = ""
        
        # TODO: check if None?
        self.window = window

        # TODO: raise exception if none (NoActions)
        actions = self._get()
        codes = Codes(_code_chars).generate(len(actions))
        self.actions = dict(zip(codes, actions))
        self._init()
    
    def _find_all_actions(self):
        None

    # def _get_all(self, accessible):
    #     try:
    #         self._actions.append(Action(accessible))
    #     except Action.NotVisible:
    #         return
    #     except Action.NoAction:
    #         None

    #     for child_accessible_index in range(accessible.get_child_count()):
    #         self._get_all(accessible.get_child_at_index(child_accessible_index))
    
    def _generate_keys(self, amount):
        # TODO: make better lol
        for index in range(len(self._actions)):
            code = chr(index+97+index%2) + chr(random.randint(97, 97+25))
            if index % 2 == 0:
                code += chr(random.randint(97, 97+25))
            if index % 3 == 0:
                code += "i"
            self._actions[index].code = code
    
    def _init(self):
        for action in self._actions:
            action.init()
    
    def match(self, code):
        return any(action.code == code for action in self._actions)

    def valid(self, code):
        return sum(1 for action in self._actions if action.valid(code))
    
    def process(self, code):
        self.code = code

        for action in self._actions:
            action.process(self.code)

    def finish(self):
        if not self.match(self.code):
            # TODO: error or something?
            return

        for action in self._actions:
            if action.code == self.code:
                action.run()
                return

class Action:
    class NotVisible(Exception):
        pass
    class NoAction(Exception):
        pass
    # TODO: implement?
    class InvalidPosition(Exception):
        pass

    _valid_accessible_states = [Atspi.StateType.VISIBLE, Atspi.StateType.SHOWING]

    def __init__(self, accessible):
        self.accessible = accessible
        self.code = None
        self._popup = None
        # TODO: figure out better way using collections

        # only bother with the visible
        states = accessible.get_state_set()
        for state in Action._valid_accessible_states:
            if not states.contains(state):
                raise Action.NotVisible

        # make sure it is actionable
        self._action = accessible.get_action_iface()
        if self._action == None:
            raise Action.NoAction
        # just checking, although pretty sure the above will error
        if self._action.get_n_actions() < 1:
            raise Action.NoAction
        # TODO: how to handle more than 1 action? (never seen)

        # TODO: this could be a cool feature to run again if a menu item
        # Actually probably need to use roles; imagine button in a button
        # if accessible.getChildCount() > 0:
        #     print("children:", accessible.getChildCount())

        component = accessible.get_component_iface()

        position = component.get_position(Atspi.CoordType.SCREEN)
        self._x, self._y = position.x, position.y

        size = component.get_size()
        self._w, self._h = size.x, size.y
        self._center_x, self._center_y = int(self._x + self._w / 2), int(self._y + self._h / 2)

        # TODO: figure out those pesky weird positions (e.g. constrain to the window)
        #raise InvalidPosition

        # TODO: determine action type
        self._type = "click"
        # self._type = "focus"
    
    def init(self):
        if self.code == None:
            return

        self._popup = Popup(self._x, self._y, self.code)
    
    def valid(self, code):
        return self.code.startswith(code)

    def process(self, code):
        if self.valid(code):
            self._popup.show(len(code))
        else:
            self._popup.hide()

    def run(self):
        if self._type == "click":
            dis = display.Display()
            root = dis.screen().root
            pointer = root.query_pointer()

            root.warp_pointer(self._center_x, self._center_y)
            dis.sync()
            time.sleep(0.001)

            ext.xtest.fake_input(dis, X.ButtonPress, 1)
            dis.sync()
            time.sleep(0.001)

            ext.xtest.fake_input(dis, X.ButtonRelease, 1)
            dis.sync()
            time.sleep(0.001)

            root.warp_pointer(pointer.root_x, pointer.root_y)
            dis.sync()

        elif self._type == "focus":
            self.accessible.grab_focus()

        # TODO: make control key held at certain times
        # it should work through x lib
