import zc.lockfile
import signal
import os
import pyatspi

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, GLib

from .config import Config

from .focus import Focus
from .mouse import Mouse
from .keys import Keys
from .overlay import Overlay

from .background import Background
from .foreground import Foreground

def new_app(raw_config):
    config = Config(raw_config)
    try:
        lock = zc.lockfile.LockFile(config.lockfile)
        return App(config, lock)
    except zc.lockfile.LockError:
        pid = int(open(config.lockfile, "r").read())
        return AppConnection(config, pid)

class AppConnection:
    def __init__(self, config: Config, pid: int):
        self._config = config
        self.pid = pid

    def foreground(self):
        os.kill(self.pid, signal.SIGUSR1)

    def background(self):
        print("already looping with pid", self.pid)
        exit(1)

class App(AppConnection):
    def __init__(self, config: Config, lock: zc.lockfile.LockFile):
        self._config = config
        self._lock = lock

        self.has_background = False
        self.has_foreground = False

        self._focus = Focus()
        self._mouse = Mouse()
        self._keys = Keys()
        self._overlay = Overlay()

        signal.signal(signal.SIGUSR1, self.remotely_trigger)

    def remotely_trigger(self, signum, frame):
        if self.has_foreground:
            return
        GLib.idle_add(self.foreground)

    def foreground(self):
        self.has_foreground = True
        with Foreground(self._config, self._focus, self._mouse, self._keys, self._overlay) as foreground:
            if foreground is not None:
                if self.has_background:
                    pyatspi.Registry.stop()
                pyatspi.Registry.start()
        self.has_foreground = False

    def background(self):
        self.has_background = True
        with Background(self._focus):
            while True:
                pyatspi.Registry.start()
        self.has_background = False