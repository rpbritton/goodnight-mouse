import logging
from typing import List

import pyatspi
import zc.lockfile

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import GLib, Gtk

from .focus import Focus
from .keys import Keys
from .mouse import Mouse

from .config import Config

from .foreground import Foreground
from .background import Background


class App:
    @staticmethod
    def new(raw_config):
        config = Config(raw_config)
        try:
            lock = zc.lockfile.LockFile(config.lockfile)
            return App(config, lock)
        except zc.lockfile.LockError:
            pid = int(open(config.lockfile, "r").read())
            logging.fatal("app already exists with pid %d", pid)

    def __init__(self, config: Config, lock: zc.lockfile.LockFile):
        self._config = config
        self._lock = lock

        # listeners
        self._focus = Focus()
        self._mouse = Mouse()
        self._keys = Keys()

        # utilities
        self._foreground = Foreground(
            self._config, self._focus, self._mouse, self._keys)
        self._background = Background(
            self._config, self._focus, self._keys)

    def __enter__(self):
        self._focus.__enter__()
        self._mouse.__enter__()
        self._keys.__enter__()

        return self

    def __exit__(self, *args):
        self._focus.__exit__(*args)
        self._mouse.__exit__(*args)
        self._keys.__exit__(*args)

    def trigger(self, flags: List[str]):
        logging.debug("triggering...")
        with self._foreground(flags) as foreground:
            foreground.start()

    def start(self):
        logging.debug("starting...")
        with self._background() as background:
            while True:
                background.start()
                self.trigger(background.flags)
