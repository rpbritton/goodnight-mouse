import pyatspi

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

        self._actions = [Action(code, accessible) for code, accessible in zip(codes, accessibles)]

    def valid(self, code):
        return any(action.valid(code) for action in self._actions)
    
    def apply(self, code):
        self.code = code

        for action in self._actions:
            action.apply(self.code)

    def match(self, code):
        return any(action.match(code) for action in self._actions)

    def do(self):
        for action in self._actions:
            if action.match(self.code):
                action.do()