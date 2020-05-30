import pyatspi
from Xlib import Xatom, display
from gi.repository import GLib
import time

from .controller import Controller
from .utils import ImmediateTimeout

FOCUS_EVENTS = ["window:activate", "window:deactivate"]

_empty_collection = pyatspi.Collection(None)
ACTIVE_WINDOW_MATCH_RULE = _empty_collection.createMatchRule(
    pyatspi.StateSet.new([pyatspi.STATE_SHOWING, pyatspi.STATE_VISIBLE, pyatspi.STATE_ACTIVE]), _empty_collection.MATCH_ALL,
    "", _empty_collection.MATCH_NONE,
    [pyatspi.ROLE_FRAME], _empty_collection.MATCH_ANY,
    "", _empty_collection.MATCH_NONE,
    False)

class FocusController(Controller):
    def __init__(self, callback):
        super().__init__()

        self.callback = callback

    def start(self):
        if not super().start(): return

        ImmediateTimeout.enable()
        pyatspi.Registry.registerEventListener(self.handle, *FOCUS_EVENTS)
        ImmediateTimeout.disable()

        # self.window = get_focused_window()

    def stop(self):
        if not super().stop(): return

        ImmediateTimeout.enable()
        try:
            pyatspi.Registry.deregisterEventListener(self.handle, *FOCUS_EVENTS)
        except GLib.Error as err:
            if err.domain != "atspi_error" or "Did not receive a reply" not in str(err.message):
                raise
        ImmediateTimeout.disable()

    def handle(self, event):
        self.callback()

def get_focused_window():
    times = []
    times.append(time.time())

    dis = display.Display()
    root_window = dis.screen().root

    times.append(time.time())
    active_window_atom = dis.get_atom("_NET_ACTIVE_WINDOW")
    active_window_property = root_window.get_property(active_window_atom, Xatom.WINDOW, 0, 1)
    active_window = dis.create_resource_object("window", active_window_property.value[0])

    times.append(time.time())
    active_pid_atom = dis.get_atom("_NET_WM_PID")
    active_pid_property = active_window.get_property(active_pid_atom, Xatom.CARDINAL, 0, 1)
    active_pid = int(active_pid_property.value[0])

    times.append(time.time())
    desktop = pyatspi.Registry.getDesktop(0)
    for application in desktop:
        times.append(time.time())
        if application.get_process_id() == active_pid:
            times.append(time.time())
            collection = application.queryCollection()
            times.append(time.time())
            active_windows = collection.getMatches(ACTIVE_WINDOW_MATCH_RULE, collection.SORT_ORDER_CANONICAL, 0, False)
            times.append(time.time())
            if len(active_windows) == 1:
                for index in range(len(times) - 1):
                    print(index, times[index + 1] - times[index])
                return active_windows[0]
            else:
                return None

    return None