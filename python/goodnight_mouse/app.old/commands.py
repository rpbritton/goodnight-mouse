import sys
import daemon
import daemon.pidfile
import signal

from .daemon import main

# import gi
# gi.require_version("Gtk", "3.0")
# gi.require_version("Gdk", "3.0")
# from gi.repository import Gtk, Gdk

# from .css import css

# # from .config import Config
# from .focus import FocusHandler
# from .action_list import ActionList
# from .keys import KeysHandler
# from .mouse import MouseHandler

import time

def start(config):
    with daemon.DaemonContext(
        # TODO: make configurable
        pidfile=daemon.pidfile.PIDLockFile("/tmp/goodnight_mouse.pid"),
        # signal_map={
        #     # signal.SIGTERM:
        # },
        stdout=sys.stdout
    ):
        print("here?")
        main(config)

def stop(config):
    None

def trigger(config):
    None
