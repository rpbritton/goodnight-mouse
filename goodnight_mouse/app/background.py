import logging
from typing import Set

import pyatspi

from .config import Config
from .focus import Focus
from .keys import Keys, KeysEvent


class Background:
    def __init__(self, config: Config, focus: Focus, keys: Keys):
        self._config = config
        self._focus = focus
        self._keys = keys

        self.triggers = []
        self.flags = set()

        self.started = False

    def __call__(self):
        return self

    def __enter__(self):
        self._focus.subscribe(self._focus_handle)
        self._keys.subscribe(self._keys_handle)

        self.triggers = []
        self.flags = set()

        self._focus_handle(self._focus.active_window)

        return self

    def __exit__(self, *args):
        self._focus.unsubscribe(self._focus_handle)

    def start(self):
        if self.started:
            return
        logging.debug("started background loop")
        self.started = True
        pyatspi.Registry.start()
        self.started = False

    def _pumpEvents(self):
        pyatspi.Registry.pumpQueuedEvents()
        return True

    def stop(self):
        if not self.started:
            return
        logging.debug("stopped background loop")
        pyatspi.Registry.stop()

    def _focus_handle(self, window: pyatspi.Accessible):
        if window is not None:
            # make sure caching/connection occurs
            window_name = window.name
            application_name = window.getApplication().name
            logging.debug("focus changed to window '%s' from application '%s'",
                          window_name, application_name)
        else:
            logging.debug("focus changed to none")

        self.triggers = self._config.window(window).triggers

    def _keys_handle(self, event: KeysEvent):
        for trigger in self.triggers:
            if trigger.key != event.key or trigger.modifiers != event.modifiers:
                continue
            if trigger.press == event.pressed:
                self.flags = trigger.flags
                self.stop()
            return True
        return False
