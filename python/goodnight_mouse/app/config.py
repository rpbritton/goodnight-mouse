import re
import pyatspi

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

STATE_LOOKUP = dict(pyatspi.StateType._enum_lookup)
STATE_LOOKUP = {**STATE_LOOKUP, **{v: k for k, v in STATE_LOOKUP.items()}}
ROLE_LOOKUP = dict(pyatspi.Role._enum_lookup)
ROLE_LOOKUP = {**ROLE_LOOKUP, **{v: k for k, v in ROLE_LOOKUP.items()}}

class Config:
    def __init__(self, raw_config):
        self.config = raw_config

        self.lockfile = raw_config["lockfile"]

        self._rules = raw_config["rules"]
        self._functions = raw_config["functions"]

    def _eval_function(self, accessible, function):
        ok = True
        if "match" in function:
            ok = eval_match(accessible, function["match"])

        if not ok:
            if "else" in function:
                return self._eval_function(accessible, function["else"])
            return None

        if "return" in function:
            return function["return"]
        if "goto" in function:
            return self._eval_function(accessible, self._functions[function["goto"]])

class WindowConfig(Config):
    def __init__(self, config, window):
        super().__init__(config.config)
        self.window = window
        self._rule = self._get_rule()
        if self._rule is None:
            return

        # TODO: check if exists/inheritance
        self.keys = self._rule["keys"]

        self.states = [STATE_LOOKUP[state] for state in self._rule["states"]]
        empty_collection = pyatspi.Collection(None)
        self.match_rule = empty_collection.createMatchRule(
            pyatspi.StateSet.new(self.states), empty_collection.MATCH_ALL,
            "", empty_collection.MATCH_NONE,
            [], empty_collection.MATCH_NONE,
            "", empty_collection.MATCH_NONE,
            False)
        self.roles = {ROLE_LOOKUP[role]: function for role, function in self._rule["roles"].items()}

        self.css = Gtk.CssProvider()
        self.css.load_from_data(self._rule["css"].encode())

    def _get_rule(self):
        for rule in self._rules:
            if "match" in rule:
                if not eval_match(self.window, rule["match"]):
                    continue
            return rule
        return None

    def _gather_actions(self, accessible):
        actions = []

        # check current accessible
        action_type = self.get_action_type(accessible)
        if action_type is not None:
            actions.append((accessible, action_type))

        # check childern
        collection = accessible.queryCollection()
        children = collection.getMatches(self.match_rule, collection.SORT_ORDER_CANONICAL, 0, False)
        for child in children:
            actions += self._gather_actions(child)

        return actions

    def get_actions(self):
        return self._gather_actions(self.window)

    def get_action_type(self, accessible):
        role = accessible.getRole()
        if role in self.roles:
            return self._eval_function(accessible, self._functions[self.roles[role]])
        return None

def match_application(accessible, regex):
    application_name = accessible.getApplication().name
    return re.search(regex, application_name) is not None

def match_action(accessible, regex):
    try:
        action = accessible.queryAction()
        if action.get_nActions() > 0:
            return re.search(regex, action.getName(0)) is not None
    except NotImplementedError:
        return False
    return False

def match_states(accessible, states):
    states = [STATE_LOOKUP[state] for state in states]
    all_states = accessible.getState().getStates()
    return all(state in all_states for state in states)

MATCH_CONDITIONS = {
    "application": match_application,
    "action": match_action,
    "states": match_states,
}

def eval_match(accessible, match):
    for submatch in match:
        ok = True
        for condition in submatch:
            if condition in MATCH_CONDITIONS:
                ok &= MATCH_CONDITIONS[condition](accessible, submatch[condition])
        if "invert" in submatch:
            ok ^= submatch["invert"]
        if not ok:
            return False
    return True