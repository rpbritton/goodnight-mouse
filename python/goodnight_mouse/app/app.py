import zc.lockfile
import signal
import os
import pyatspi

import gi
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, Atspi

from .config import Config
from .revealer import RevealerController
from .overlay import OverlayController
from .registry import RegistryController

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
        self.registry_controller = RegistryController()
        self.overlay_controller = OverlayController()

        signal.signal(signal.SIGUSR1, lambda *args: None)

    def loop(self):
        """Start this app as the background, running forever until signal."""
        self.registry_controller.start()

        while True:
            print("waiting")
            self.wait()
            print("triggered")
            self.trigger()

    def wait(self):
        """Wait for a signal to continue"""
        # make sure no gtk events are waiting before "hibernatng"
        while Gtk.events_pending():
            Gtk.main_iteration()

        signal.pause()

    def trigger(self):
        """Start a new controller."""
        revealer_controller = RevealerController(self.config, self.registry_controller.get_focused_window(), self.overlay_controller)
        revealer_controller.start()

    def stop(self):
        self.registry_controller.stop()