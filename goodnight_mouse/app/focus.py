import logging

import pyatspi
from gi.repository import GLib
from Xlib import Xatom, display

from .subscription import Subscription
from .utils import ImmediateTimeout, EMPTY_COLLECTION


class Focus(Subscription):
    _ACTIVATE_EVENTS = ["window:activate"]
    _DEACTIVATE_EVENTS = ["window:deactivate"]

    _ACTIVE_WINDOW_MATCH_RULE = EMPTY_COLLECTION.createMatchRule(
        pyatspi.StateSet.new([pyatspi.STATE_SHOWING, pyatspi.STATE_VISIBLE,
                              pyatspi.STATE_ACTIVE]), EMPTY_COLLECTION.MATCH_ALL,
        "", EMPTY_COLLECTION.MATCH_NONE,
        [], EMPTY_COLLECTION.MATCH_NONE,
        "", EMPTY_COLLECTION.MATCH_NONE,
        False)

    def __init__(self):
        super().__init__()

        self._active_window = None

    def __enter__(self):
        logging.debug("registering focus listener")

        ImmediateTimeout.enable()
        pyatspi.Registry.registerEventListener(
            self._activate_handle, *self._ACTIVATE_EVENTS)
        pyatspi.Registry.registerEventListener(
            self._deactivate_handle, *self._DEACTIVATE_EVENTS)
        ImmediateTimeout.disable()

        self.active_window = self.fresh_active_window()

        return self

    def __exit__(self, *args):
        logging.debug("deregistering focus listener")

        pyatspi.Registry.deregisterEventListener(
            self._activate_handle, *self._ACTIVATE_EVENTS)
        pyatspi.Registry.deregisterEventListener(
            self._deactivate_handle, *self._DEACTIVATE_EVENTS)

    def _activate_handle(self, event):
        self.active_window = event.source

    def _deactivate_handle(self, event):
        if event.source is not self.active_window:
            return
        self.active_window = None

    @property
    def active_window(self):
        return self._active_window

    @active_window.setter
    def active_window(self, window: pyatspi.Accessible = None):
        if window is self.active_window:
            return
        self._active_window = window
        self.notify(self.active_window)

    @classmethod
    def fresh_active_window(cls):
        dis = display.Display()
        root_window = dis.screen().root

        active_window_atom = dis.get_atom("_NET_ACTIVE_WINDOW")
        active_window_property = root_window.get_property(
            active_window_atom, Xatom.WINDOW, 0, 1)
        active_window = dis.create_resource_object(
            "window", active_window_property.value[0])

        active_pid_atom = dis.get_atom("_NET_WM_PID")
        active_pid_property = active_window.get_property(
            active_pid_atom, Xatom.CARDINAL, 0, 1)
        active_pid = int(active_pid_property.value[0])

        desktop = pyatspi.Registry.getDesktop(0)
        for application in desktop:
            if application.get_process_id() == active_pid:
                collection = application.queryCollection()
                active_windows = collection.getMatches(
                    cls._ACTIVE_WINDOW_MATCH_RULE, collection.SORT_ORDER_CANONICAL, 0, False)
                if len(active_windows) == 1:
                    return active_windows[0]
                else:
                    return None

        return None
