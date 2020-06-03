import logging
import os
import signal

import pyatspi
import zc.lockfile

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import GLib, Gtk

from .background import Background
from .config import Config
from .focus import Focus
from .foreground import Foreground
from .keys import Keys
from .mouse import Mouse
from .overlay import Overlay


class AppConnection:
    def __init__(self, config: Config, pid: int):
        self._config = config
        self.pid = pid

    def __enter__(self):
        return self

    def __exit__(self, *args):
        pass

    def foreground(self):
        logging.debug("sending signal to background process...")
        os.kill(self.pid, signal.SIGUSR1)

    def background(self):
        logging.error("already running in background with pid %d")
        exit(1)


class App(AppConnection):
    @staticmethod
    def new(raw_config):
        config = Config(raw_config)
        try:
            lock = zc.lockfile.LockFile(config.lockfile)
            return App(config, lock)
        except zc.lockfile.LockError:
            pid = int(open(config.lockfile, "r").read())
            logging.debug("app already exists with pid %d, connecting...", pid)
            return AppConnection(config, pid)

    def __init__(self, config: Config, lock: zc.lockfile.LockFile):
        self._config = config
        self._lock = lock

        self._prev_signal_handlers = dict()

        self.has_background = False
        self.has_foreground = False

        self._focus = Focus()
        self._mouse = Mouse()
        self._keys = Keys()
        self._overlay = Overlay()

    def __enter__(self):
        self._focus.__enter__()
        self._mouse.__enter__()
        self._keys.__enter__()

        self._prev_signal_handlers[signal.SIGUSR1] = signal.signal(
            signal.SIGUSR1, self.remotely_trigger)

        return self

    def __exit__(self, *args):
        self._focus.__exit__(*args)
        self._mouse.__exit__(*args)
        self._keys.__exit__(*args)

        for signal_num, signal_handler in self._prev_signal_handlers.items():
            signal.signal(signal_num, signal_handler)

    def remotely_trigger(self, *args):
        logging.debug("remotely triggered")
        GLib.idle_add(self.foreground)

    def foreground(self):
        if self.has_foreground:
            logging.error("already running foreground")
            return
        self.has_foreground = True
        logging.debug("starting foreground...")
        with Foreground(self._config, self._focus, self._mouse, self._keys, self._overlay) as foreground:
            if foreground is not None:
                if self.has_background:
                    pyatspi.Registry.stop()
                logging.debug("started foreground loop")
                pyatspi.Registry.start()
                logging.debug("finished foreground loop")
        self.has_foreground = False

    def background(self):
        if self.has_background:
            return
        self.has_background = True
        logging.debug("starting background...")
        with Background(self._focus) as background:
            if background is not None:
                while True:
                    logging.debug("started background loop")
                    pyatspi.Registry.start()
                    logging.debug("restarting background loop...")
        self.has_background = False
