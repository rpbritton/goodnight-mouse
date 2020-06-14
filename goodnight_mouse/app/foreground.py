import logging
from typing import List, Set

import pyatspi

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

from .focus import Focus
from .keys import Keys, KeysEvent
from .mouse import Mouse

from .actions import Actions
from .config import Config
from .overlay import Overlay


class Foreground:
    def __init__(self, config: Config, focus: Focus, mouse: Mouse, keys: Keys):
        self._config = config
        self._focus = focus
        self._mouse = mouse
        self._keys = keys

        self.overlay = Overlay()
        self.actions = Actions(self.overlay)

        self._window_config = self._config.window()

        self.flags = set()
        self.code = []

        self.started = False

    def __call__(self, flags: Set[str] = None):
        if flags is None:
            flags = set()
        self.flags = flags

        return self

    def __enter__(self):
        self._focus.subscribe(self._handle_focus)
        self._mouse.subscribe(self._handle_mouse)
        self._keys.subscribe(self._handle_keys)

        active_window = self._focus.active_window
        self._window_config = self._config.window(active_window)

        self.overlay(self._window_config).__enter__()
        self.actions(self._window_config, self.flags).__enter__()

        self.code = []

        return self

    def __exit__(self, *args):
        self.actions.__exit__(*args)
        self.overlay.__exit__(*args)

        self._keys.unsubscribe(self._handle_keys)
        self._mouse.unsubscribe(self._handle_mouse)
        self._focus.unsubscribe(self._handle_focus)

    def _handle_keys(self, event: KeysEvent):
        logging.debug("handling key '%s', pressed: %s",
                      Gdk.keyval_name(event.key), (event.pressed))

        # only handle presses
        if event.pressed is not True:
            return True

        # keys to look for
        changed = False
        if event.key == Gdk.KEY_Escape:
            self.stop()
            return True
        if event.key == Gdk.KEY_BackSpace:
            if self.code:
                self.code = self.code[:-1]
                changed = True
        if event.key in self._window_config.characters:
            self.code.append(event.key)
            changed = True
        if not changed:
            return True

        # check the validity of the new code
        num_valid_actions = self.actions.matches(self.code)

        # check if there is only one action left
        if num_valid_actions == 1:
            self.stop()
            self.actions.execute(self.code)
            return True

        # reset if no actions left
        if num_valid_actions == 0:
            self.code = []
        self.actions.code = self.code

        # returning true means the key is absorbed
        return True

    def _handle_mouse(self):
        logging.debug("handling mouse event")

        self.stop()

    def _handle_focus(self, window: pyatspi.Accessible):
        logging.debug("handling focus change")

        self.stop()

    def start(self):
        if self._window_config.accessible is None:
            return

        logging.debug("started foreground loop")
        self.started = True
        pyatspi.Registry.start()
        self.started = False

        # do the remaining action if exists
        # num_valid_actions = self.actions.matches(self.code)
        # if num_valid_actions == 1:
        #     self.actions.execute(self.code)

    def stop(self):
        if not self.started:
            return
        logging.debug("stopped foreground loop")
        pyatspi.Registry.stop()
