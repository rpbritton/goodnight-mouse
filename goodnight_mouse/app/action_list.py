import pyatspi

from .codes import Codes
from .action import ACTION_MAP

import time

def recurse(parent):
    for child in parent:
        states = child.getState()
        if states.contains(pyatspi.STATE_SHOWING):
            recurse(child)

class ActionList:
    def __init__(self, config, window):
        times = []
        times.append(time.time())

        self.current_code = ""
        self.config = config
        self.window = window

        recurse(window)
        times.append(time.time())

        # TODO: check return?
        collection = self.window.queryCollection()
        rule = collection.createMatchRule(
            config.get_states(), collection.MATCH_ALL,
            "", collection.MATCH_NONE,
            config.get_roles(), collection.MATCH_ANY,
            "", collection.MATCH_NONE,
            False)
        accessibles = collection.getMatches(rule, collection.SORT_ORDER_CANONICAL, 0, True)
        # TODO: check if len(accessibles) == 0
        times.append(time.time())

        action_types = []
        for accessible in accessibles[:]:
            action_type = config.get_action_type(accessible)
            if action_type == None:
                accessibles.remove(accessible)
            else:
                action_types.append(action_type)
        times.append(time.time())

        codes = Codes(config.get_keys()).generate(len(accessibles))
        times.append(time.time())

        self.actions = []
        for code, accessible, action_type in zip(codes, accessibles, action_types):
            self.actions.append(ACTION_MAP[action_type](code, accessible))
        times.append(time.time())

        for index in range(len(times) - 1):
            print("index", index, "time", times[index+1] - times[index])

    def valid(self, code):
        return any(action.valid(code) for action in self.actions)

    def apply(self, code):
        self.code = code

        for action in self.actions:
            action.apply(self.code)

    def match(self, code):
        return any(action.match(code) for action in self.actions)

    def do(self):
        for action in self.actions:
            if action.match(self.code):
                action.do()
                return