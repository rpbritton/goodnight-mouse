import zc.lockfile
import signal
import os
import pyatspi

from .config import Config
from .registry import Registry
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
    def __init__(self, config, pid):
        self.config = config
        self.pid = pid

    def trigger(self):
        os.kill(self.pid, signal.SIGUSR1)

class App(AppConnection):
    def __init__(self, config, lock):
        self.config = config
        self.lock = lock
        self.controller = None

        self.registry = Registry(config)

        signal.signal(signal.SIGUSR1, self._remotely_trigger)

    def _remotely_trigger(self, signum, frame):
        self.remotely_trigger()
    def remotely_trigger(self):
        """Trigger this running app."""
        self.start_controller()

    def trigger(self):
        """To trigger a non-running app, exiting after."""
        self.start_controller()
        self.run_cycle()

    def start_background(self):
        """This is a running app, which runs forever."""
        self.registry.refresh_all()
        while True:
            self.run_cycle()

    def run_cycle(self):
        """Wait for atspi events, and the controller will
        kill the loop to signal it's end.
        """
        pyatspi.Registry.start()
        self.stop_controller()

    def stop_controller(self):
        """Stop the current controller."""
        if self.controller:
            self.controller.end()
            self.controller = None
            return True
        return False

    def start_controller(self):
        """Start a new controller, or abort current controller."""
        if self.stop_controller():
            return
        actions = self.registry.get_actions()
        if len(actions) < 1:
            return
        self.controller = Controller(self.config, actions)

