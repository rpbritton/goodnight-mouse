import zc.lockfile
import signal
import os
import pyatspi
import time

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, GLib

from .config import Config
from .background import BackgroundController
from .foreground import ForegroundController

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
        self.config = config
        self.pid = pid

    def trigger(self):
        os.kill(self.pid, signal.SIGUSR1)

    def loop(self):
        print("already looping with pid", self.pid)
        exit(1)

class App(AppConnection):
    def __init__(self, config: Config, lock: zc.lockfile.LockFile):
        self.config = config
        self.lock = lock
        self.background_controller = BackgroundController()
        self.foreground_controller = ForegroundController(self.config, self.background_controller)

        signal.signal(signal.SIGUSR1, self._remotely_trigger)

    def loop(self):
        self.background_controller.start()
        pyatspi.Registry.start()

    def _remotely_trigger(self, signum, frame):
        self.remotely_trigger()
    def remotely_trigger(self):
        if self.foreground_controller.is_running():
            return
        GLib.idle_add(self.trigger)

    def trigger(self):
        self.foreground_controller.start()
        pyatspi.Registry.start()

    def stop(self):
        self.background_controller.stop()
        self.foreground_controller.stop()