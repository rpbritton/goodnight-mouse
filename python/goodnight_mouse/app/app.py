import zc.lockfile
import signal
import os
import time
from gi.repository import GLib

from .config import Config
from .overlay import Overlay
from .controller import Controller

def new_app(raw_config):
    config = Config(raw_config)
    try:
        lock = zc.lockfile.LockFile(config.lockfile)
        return App(config, lock)
    except zc.lockfile.LockError:
        pid = int(open(config.lockfile, "r").read())
        return AppConnection(config, pid)

class AppConnection:
    def __init__(self, config: Config, pid):
        self.config = config
        self.pid = pid

    def trigger(self):
        os.kill(self.pid, signal.SIGUSR1)

    def loop(self):
        print("already looping with pid", self.pid)
        exit(1)

class App(AppConnection):
    def __init__(self, config: Config, lock):
        self.config = config
        self.lock = lock
        self.overlay = Overlay()

        signal.signal(signal.SIGUSR1, lambda *args: None)

    def loop(self):
        """Start this app as the background, running forever until signal."""
        while True:
            print("waiting")
            self.wait()
            print("triggered")
            self.trigger()

    def wait(self):
        """Wait for a signal to continue"""
        signal.pause()

    def trigger(self):
        """Start a new controller."""
        controller = Controller(self.config, self.overlay)
        controller.start()