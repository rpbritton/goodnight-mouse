# import random
import pyatspi

# from Xlib import X, display, ext

# import gi
# gi.require_version("Gtk", "3.0")
# gi.require_version("Gdk", "3.0")
# from gi.repository import Gtk, Gdk

from .codes import Codes
from .action import Action

# TODO: put in config
_code_chars = "fjdkghsla"

_match_states = pyatspi.StateSet.new([pyatspi.STATE_VISIBLE, pyatspi.STATE_SHOWING])
_match_roles = list(Action.ROLES)

class ActionList:
    def __init__(self, window):
        self.current_code = ""
        self.window = window
    
        collection = self.window.queryCollection()
        rule = collection.createMatchRule(
            _match_states, collection.MATCH_ALL,
            "", collection.MATCH_NONE,
            _match_roles, collection.MATCH_ANY,
            "", collection.MATCH_NONE,
            False)
        accessibles = collection.getMatches(rule, collection.SORT_ORDER_CANONICAL, 0, True)

        # TODO: check if len(accessibles) == 0

        codes = Codes(_code_chars).generate(len(accessibles))

        self.actions = [Action(code, accessible) for code, accessible in zip(codes, accessibles)]
    
#     def _init(self):
#         for action in self._actions:
#             action.init()
    
#     def match(self, code):
#         return any(action.code == code for action in self._actions)

#     def valid(self, code):
#         return sum(1 for action in self._actions if action.valid(code))
    
#     def process(self, code):
#         self.code = code

#         for action in self._actions:
#             action.process(self.code)

#     def finish(self):
#         if not self.match(self.code):
#             # TODO: error or something?
#             return

#         for action in self._actions:
#             if action.code == self.code:
#                 action.run()
#                 return
