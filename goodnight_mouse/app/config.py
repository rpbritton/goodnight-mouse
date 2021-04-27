from collections import namedtuple
import logging
import re
from string import Template
from typing import List, Tuple, Set, Dict

import pyatspi

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gdk, Gtk

from .utils import EMPTY_COLLECTION


class TriggerConfig:
    def __init__(self, properties: dict):
        self._properties = properties

    @ property
    def modifiers(self) -> int:
        if "modifiers" not in self._properties:
            return 0

        return ConfigParser.modifier_mask(self._properties["modifiers"])

    @ property
    def key(self) -> int:
        if "key" not in self._properties:
            return Gdk.KEY_VoidSymbol

        return ConfigParser.keysym(self._properties["key"])

    @ property
    def press(self) -> bool:
        if "press" not in self._properties:
            return True

        return self._properties["press"]

    @ property
    def flags(self) -> Set[str]:
        if "flags" not in self._properties:
            return []

        return set(self._properties["flags"])


class ExecuteConfig:
    @classmethod
    def new(cls, properties: dict):
        if "type" not in properties or properties["type"] not in cls.types:
            return cls(properties)

        return cls.types[properties["type"]](properties)

    def __init__(self, properties: dict):
        if "type" in properties and properties["type"] in self.types:
            self.type = properties["type"]
        else:
            self.type = None

    types = {}


class ExecuteActionConfig(ExecuteConfig):
    def __init__(self, properties: dict):
        super().__init__(properties)

        if "match" in properties:
            self.match = properties["match"]
        else:
            self.match = '$^'


class ExecuteKeyConfig(ExecuteConfig):
    def __init__(self, properties: dict):
        super().__init__(properties)

        if "key" in properties:
            self.key = ConfigParser.keysym(properties["key"])
        else:
            self.key = Gdk.KEY_VoidSymbol

        if "action" in properties:
            self.action = properties["action"]
        else:
            self.action = ""

        if "modifiers" in properties:
            self.modifiers = ConfigParser.modifier_mask(
                properties["modifiers"])
        else:
            self.modifiers = 0


class ExecuteMouseConfig(ExecuteConfig):
    def __init__(self, properties: dict):
        super().__init__(properties)

        if "button" in properties:
            self.button = properties["button"]
        else:
            self.key = 0

        if "action" in properties:
            self.action = properties["action"]
        else:
            self.action = ""

        if "modifiers" in properties:
            self.modifiers = ConfigParser.modifier_mask(
                properties["modifiers"])
        else:
            self.modifiers = 0


class ExecuteFocusConfig(ExecuteConfig):
    def __init__(self, properties: dict):
        super().__init__(properties)


ExecuteConfig.types = {
    "action": ExecuteActionConfig,
    "key": ExecuteKeyConfig,
    "mouse": ExecuteMouseConfig,
    "focus": ExecuteFocusConfig,
}


class ActionConfig:
    def __init__(self, accessible: pyatspi.Accessible, properties: dict):
        self.accessible = accessible
        self._properties = properties

    @ property
    def css(self) -> Gtk.CssProvider:
        if "css" not in self._properties:
            return ConfigParser.css("")

        return ConfigParser.css(self._properties["css"])

    @ property
    def x(self) -> Template:
        if "x" not in self._properties:
            return Template("$x")

        return Template(self._properties["x"])

    @ property
    def y(self) -> Template:
        if "y" not in self._properties:
            return Template("$y")

        return Template(self._properties["y"])

    @property
    def execute(self) -> List[dict]:
        if "execute" not in self._properties:
            return []

        executions = []
        for properties in self._properties["execute"]:
            execution = ExecuteConfig.new(properties)
            if execution is not None:
                executions.append(execution)
        return executions


class WindowConfig:
    def __init__(self, window: pyatspi.Accessible = None, rules: List[dict] = None):
        self.accessible = window
        self._properties = {}

        if rules is None:
            roles = []

        if self.accessible is not None:
            for rule in reversed(rules):
                if ConfigParser.match(rule["condition"], window):
                    self._properties = rule
                    break

    @property
    def css(self) -> Gtk.CssProvider:
        if "css" not in self._properties:
            return ConfigParser.css("")

        return ConfigParser.css(self._properties["css"])

    @property
    def characters(self) -> List[int]:
        if "characters" not in self._properties:
            return []

        return [ConfigParser.keysym(character) for character in self._properties["characters"]]

    @property
    def triggers(self) -> List[TriggerConfig]:
        if "triggers" not in self._properties:
            return []

        return [TriggerConfig(trigger) for trigger in self._properties["triggers"]]

    @property
    def states(self) -> List[int]:
        if "states" not in self._properties:
            return []

        return [ConfigParser.state(state) for state in self._properties["states"]]

    @property
    def roles(self) -> Dict[int, dict]:
        if "roles" not in self._properties:
            return []

        return {ConfigParser.role(role): function for role, function in self._properties["roles"].items()}

    def actions(self, flags: Set[str] = None) -> List[ActionConfig]:
        if self.accessible is None:
            return []

        if flags is None:
            flags = {}
        match_rule = EMPTY_COLLECTION.createMatchRule(
            pyatspi.StateSet.new(self.states), EMPTY_COLLECTION.MATCH_ALL,
            "", EMPTY_COLLECTION.MATCH_NONE,
            [], EMPTY_COLLECTION.MATCH_NONE,
            "", EMPTY_COLLECTION.MATCH_NONE,
            False)
        roles = self.roles

        actions = []
        accessibles = [self.accessible]
        while accessibles:
            accessible = accessibles.pop()

            rule = {
                "type": "rule",
                "check_children": True,
            }

            # check current accessible
            role = accessible.getRole()
            if role in roles:
                properties_list = ConfigParser.function(
                    roles[role], accessible, flags)
                if properties_list is not None:
                    for properties in properties_list:
                        if "type" not in properties:
                            continue

                        if properties["type"] == "action":
                            actions.append(ActionConfig(
                                accessible, properties))
                        elif properties["type"] == "rule":
                            rule = {**rule, **properties}

            # check childern
            if not rule["check_children"]:
                continue
            collection = accessible.queryCollection()
            accessibles += collection.getMatches(
                match_rule, collection.SORT_ORDER_CANONICAL, 0, False)

        return actions


class Config:
    def __init__(self, config: dict):
        self._config = config

    @ property
    def lockfile(self) -> str:
        return self._config["lockfile"]

    def window(self, window: pyatspi.Accessible = None) -> WindowConfig:
        # TODO: add caching
        return WindowConfig(window, self._config["rules"])


class ConfigParser:
    @ classmethod
    def state(cls, name: str) -> int:
        value = getattr(pyatspi, "STATE_" + name.upper(), None)
        if value is None:
            logging.fatal("unknown state '%s'", name)
        return value

    @ classmethod
    def role(cls, name: str) -> int:
        value = getattr(pyatspi, "ROLE_" + name.upper(), None)
        if value is None:
            logging.fatal("unknown modifier '%s'", name)
        return value

    @ classmethod
    def modifier(cls, name: str) -> int:
        value = getattr(Gdk.ModifierType, name.upper() + "_MASK", None)
        if value is None:
            logging.fatal("unknown modifier '%s'", name)
        return value

    @ classmethod
    def modifier_mask(cls, names: List[str]) -> int:
        modifiers = 0

        keymap = Gdk.Keymap.get_default()
        for name in names:
            modifier = cls.modifier(name)
            modifiers |= modifier

            success, modifier = keymap.map_virtual_modifiers(modifier)
            if success:
                modifiers |= modifier

        return modifiers & 0xFF

    @ classmethod
    def keysym(cls, name: str) -> int:
        value = Gdk.keyval_from_name(name)
        if value == Gdk.KEY_VoidSymbol:
            logging.fatal("unknown character '%s'", name)
        return value

    @ classmethod
    def css(cls, css: str) -> Gtk.CssProvider:
        css_provider = Gtk.CssProvider()
        css_provider.load_from_data(css.encode())
        return css_provider

    @ classmethod
    def function(cls, function: dict, accessible: pyatspi.Accessible, flags: Set[str] = None):
        passed = True
        if "condition" in function:
            passed = cls.match(function["condition"], accessible, flags)

        if passed is True:
            if "return" in function:
                return function["return"]
            elif "then" in function:
                return cls.function(function["then"], accessible, flags)
        elif "else" in function:
            return cls.function(function["else"], accessible, flags)

        return None

    @ classmethod
    def match(cls, match: List[dict], accessible: pyatspi.Accessible, flags: Set[str] = None) -> bool:
        if flags is None:
            flags = {}

        for submatch in match:
            matches = True

            if "application" in submatch:
                matches &= cls._application(
                    accessible, submatch["application"])
            if "name" in submatch:
                matches &= cls._name(
                    accessible, submatch["name"])
            if "action" in submatch:
                matches &= cls._action(
                    accessible, submatch["action"])
            if "states" in submatch:
                matches &= cls._states(
                    accessible, submatch["states"])
            if "role" in submatch:
                matches &= cls._role(
                    accessible, submatch["role"])
            if "flags" in submatch:
                matches &= set(submatch["flags"]).issubset(flags)
            if "invert" in submatch:
                matches ^= submatch["invert"]

            if not matches:
                return False

        return True

    @ classmethod
    def _application(cls, accessible: pyatspi.Accessible, regex: str) -> bool:
        application_name = accessible.getApplication().name
        return re.search(regex, application_name) is not None

    @ classmethod
    def _name(cls, accessible: pyatspi.Accessible, regex: str) -> bool:
        return re.search(regex, accessible.name) is not None

    @ classmethod
    def _action(cls, accessible: pyatspi.Accessible, regex: str) -> bool:
        try:
            action = accessible.queryAction()
            for index in range(action.get_nActions()):
                if re.search(regex, action.getName(index)) is not None:
                    return True
        except NotImplementedError:
            return False

        return False

    @ classmethod
    def _states(cls, accessible: pyatspi.Accessible, states: Set[str]):
        existing_states = set(accessible.getState().getStates())
        return {ConfigParser.state(state) for state in states}.issubset(existing_states)

    @ classmethod
    def _role(cls, accessible: pyatspi.Accessible, role: str):
        return accessible.getRole() == ConfigParser.role(role)
