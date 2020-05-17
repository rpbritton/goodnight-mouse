import re
import pyatspi

STATE_LOOKUP = dict(pyatspi.StateType._enum_lookup)
STATE_LOOKUP = {**STATE_LOOKUP, **{v: k for k, v in STATE_LOOKUP.items()}}
ROLE_LOOKUP = dict(pyatspi.Role._enum_lookup)
ROLE_LOOKUP = {**ROLE_LOOKUP, **{v: k for k, v in ROLE_LOOKUP.items()}}

class Config:
    def __init__(self, raw_config, window):
        self.config = raw_config
        application_name = window.getApplication().name

        for rule in self.config["rules"]:
            conditions = rule["conditions"]
            if "application" in conditions:
                if re.search(conditions["application"], application_name) == None:
                    continue

            self.rule = rule
            break

        # TODO: use inheritance to assemble "complete" rule

    def get_states(self):
        if "states" not in self.rule:
            return []

        return pyatspi.StateSet.new([STATE_LOOKUP[state] for state in self.rule["states"]])

    def get_roles(self):
        if "roles" not in self.rule:
            return []

        return [ROLE_LOOKUP[role] for role in self.rule["roles"]]

    def get_keys(self):
        if "popups" in self.rule and "keys" in self.rule["popups"]:
            return self.rule["popups"]["keys"]

    def get_action_type(self, accessible):
        role = ROLE_LOOKUP[accessible.getRole()]
        if "roles" not in self.rule or role not in self.rule["roles"]:
            return None

        return self._evaluate_function_name(accessible, self.rule["roles"][role])

    def _evaluate_function_name(self, accessible, function_name):
        if "functions" not in self.rule or function_name not in self.rule["functions"]:
            return None

        return self._evaluate_function(accessible, self.rule["functions"][function_name])

    def _evaluate_function(self, accessible, function):
        if "match" in function and not self._evaluate_match(accessible, function["match"]):
            if "else" in function:
                return self._evaluate_function(accessible, function["else"])
            else:
                return None

        if "do" in function:
            return function["do"]

        if "then" in function:
            return self._evaluate_function_name(accessible, function["then"])

    def _evaluate_match(self, accessible, match_rules):
        for rule in match_rules:
            invert = False
            if "invert" in rule:
                invert = rule["invert"]

            if "action" in rule:
                try:
                    action = accessible.queryAction()
                    if not invert ^ (
                        action.get_nActions() > 0
                        and re.search(rule["action"], action.getName(0)) != None
                    ):
                        return False
                except NotImplementedError:
                    if not invert:
                        return False

            if "states" in rule:
                states = accessible.getState()
                result = all(states.contains(STATE_LOOKUP[state]) for state in rule["states"])
                if not invert ^ result:
                    return False

        return True