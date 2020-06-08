import logging
import re
from typing import List, Tuple, Set

import pyatspi

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gdk, Gtk

from .focus import Focus


class TriggerConfig:
    def __init__(self, trigger: dict):
        self._modifiers = 0
        keymap = Gdk.Keymap.get_default()
        for modifier_name in Parser.attribute(trigger, "modifiers"):
            modifier = Parser.modifier(modifier_name)
            self._modifiers |= modifier

            success, real_modifier = keymap.map_virtual_modifiers(
                modifier)
            if success:
                self._modifiers |= real_modifier

        self._hotkey = Gdk.keyval_from_name(
            Parser.attribute(trigger, "hotkey"))

        self._flags = Parser.attribute(trigger, "flags")

    @ property
    def modifiers(self) -> int:
        return self._modifiers

    @ property
    def hotkey(self) -> int:
        return self._hotkey

    @ property
    def flags(self) -> List[str]:
        return self._flags


class WindowConfig:
    def __init__(self, window: pyatspi.Accessible, rules: dict):
        self._accessible = window

        self._rule = None
        if self._accessible is not None:
            for rule in rules:
                match = Parser.attribute(rule, "condition")
                if Parser.match(match, window):
                    self._rule = rule
                    break

        self._characters = []
        self._css = Gtk.CssProvider()
        self._triggers = []
        self._states = {}
        self._roles = {}
        if self._rule is not None:
            self._characters = [Parser.keysym(
                character) for character in Parser.attribute(self._rule, "characters")]

            self._css = Parser.css(Parser.attribute(self._rule, "css"))

            self._triggers = [TriggerConfig(
                trigger) for trigger in Parser.attribute(self._rule, "triggers")]

            self._states = {Parser.state(
                state) for state in Parser.attribute(self._rule, "states")}

            self._roles = {Parser.role(role)
                           for role in Parser.attribute(self._rule, "roles")}

    @ property
    def accessible(self) -> pyatspi.Accessible:
        return self._accessible

    @ property
    def characters(self) -> List[int]:
        return self._characters

    @ property
    def css(self) -> Gtk.CssProvider:
        return self._css

    @ property
    def triggers(self) -> List[TriggerConfig]:
        return self._triggers

    @ property
    def states(self) -> Set[int]:
        return self._states

    @ property
    def roles(self) -> Set[int]:
        return self._roles


class ActionConfig:
    @ classmethod
    def get_actions(cls, window_config: WindowConfig, flags: Set[str]):
        if window_config is None:
            return []

        empty_collection = pyatspi.Collection(None)
        match_rule = empty_collection.createMatchRule(
            pyatspi.StateSet.new(
                window_config.states), empty_collection.MATCH_ALL,
            "", empty_collection.MATCH_NONE,
            [], empty_collection.MATCH_NONE,
            "", empty_collection.MATCH_NONE,
            False)
        roles = window_config.roles

        actions = []
        accessibles = [window_config.accessible]
        while len(accessibles) > 0:
            accessible = accessibles.pop()

            # check current accessible
            role = accessible.getRole()
            if role in window_config.roles:
                action_properties = Parser.function(
                    window_config.roles[role], accessible, flags)
                if action_properties is not None:
                    actions.append(cls(accessible, action_properties))

            # check childern
            collection = accessible.queryCollection()
            accessibles += collection.getMatches(
                match_rule, collection.SORT_ORDER_CANONICAL, 0, False)

        return actions

    def __init__(self, accessible: pyatspi.Accessible, action_properties: dict):
        self._accessible = accessible
        self._css = Parser.css(Parser.attribute(
            action_properties, "css"))
        self._position = tuple(Parser.attribute(action_properties, "position"))
        self._action = Parser.attribute(action_properties, "do")

    @ property
    def accessible(self):
        return self._accessible

    @ property
    def css(self) -> Gtk.CssProvider:
        return self._css

    @ property
    def position(self) -> Tuple[int, int]:
        return self._position

    def action(self) -> str:
        return self._action


class Config:  # TODO: implement subscription for changes
    def __init__(self, config: dict):
        self._config = config
        self._focus = None

        self._lockfile = Parser.attribute(self._config, "lockfile")
        self._rules = Parser.attribute(self._config, "rules")

        self._window = WindowConfig(None, self._rules)

    def __call__(self, focus: Focus):
        self._focus = focus

        return self

    def __enter__(self):
        if self._focus is None:
            return None

        self._focus.subscribe(self._handle_focus)
        self._handle_focus(self._focus.get_active_window())

        return self

    def __exit__(self, *args):
        self._focus.unsubscribe(self._handle_focus)

    @property
    def lockfile(self) -> str:
        return self._lockfile

    @property
    def window(self) -> WindowConfig:
        return self._window

    @property
    def actions(self, flags) -> List[ActionConfig]:
        return ActionConfig.get_actions(self._window, flags)

    def _handle_focus(self, window: pyatspi.Accessible):
        self._window = WindowConfig(window, self._rules)


class Parser:
    @classmethod
    def attribute(cls, dictionary: dict, attribute: str):
        if attribute in dictionary:
            return dictionary[attribute]
        else:
            logging.fatal("missing %s in %r", attribute, dictionary)

    @classmethod
    def state(cls, name: str) -> int:
        value = getattr(pyatspi, "STATE_" + name.upper(), None)
        if value is None:
            logging.fatal("unknown state '%s'", name)
        return value

    @classmethod
    def role(cls, name: str) -> int:
        value = getattr(pyatspi, "ROLE_" + name.upper(), None)
        if value is None:
            logging.fatal("unknown modifier '%s'", name)
        return value

    @classmethod
    def modifier(cls, name: str) -> int:
        value = getattr(Gdk.ModifierType, name.upper() + "_MASK", None)
        if value is None:
            logging.fatal("unknown modifier '%s'", name)
        return value

    @classmethod
    def keysym(cls, name: str) -> int:
        value = Gdk.keyval_from_name(name)
        if value == Gdk.KEY_VoidSymbol:
            logging.fatal("unknown character '%s'", name)
        return value

    @classmethod
    def css(cls, css: str) -> Gtk.CssProvider:
        css_provider = Gtk.CssProvider()
        css_provider.load_from_data(css.encode())
        return css_provider

    @classmethod
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

    @classmethod
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
                matches &= submatch["flags"].issubset(flags)
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
            if action.get_nActions() > 0:
                return re.search(regex, action.getName(0)) is not None
        except NotImplementedError:
            return False

        return False

    @ classmethod
    def _states(cls, accessible: pyatspi.Accessible, states: Set[str]):
        existing_states = set(accessible.getState().getStates())
        return {Parser.state(state) for state in states}.issubset(existing_states)

    @ classmethod
    def _role(cls, accessible: pyatspi.Accessible, role: str):
        return accessible.getRole() == Parser.role(role)
