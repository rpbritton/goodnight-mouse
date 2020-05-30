import pyatspi
import time

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

from .config import WindowConfig
# from .focus import get_focused_window, get_applications
from .action import new_action

class CachedWindow:
    def __init__(self, config, window):
        self.config = WindowConfig(config, window)
        self.window = window
        self.id = window.accessibleId
        self.actions = {}

        # self.refresh()

    # def refresh(self):
    #     collection = self.window.queryCollection()
    #     rule = collection.createMatchRule(
    #         pyatspi.StateSet.new(self.config.states), collection.MATCH_ALL,
    #         "", collection.MATCH_NONE,
    #         [role for role in self.config.roles], collection.MATCH_ANY,
    #         "", collection.MATCH_NONE,
    #         False)
    #     # TODO: is there a better order?
    #     accessibles = collection.getMatches(rule, collection.SORT_ORDER_CANONICAL, 0, True)
    #     self.actions = []
    #     start = time.time()
    #     for accessible in accessibles:
    #         action_type = self.config.get_action_type(accessible)
    #         if action_type is not None:
    #             self.actions.append(new_action(self.config, accessible, action_type))
    #     end = time.time()
    #     print(end - start)

        # time.sleep(1)

        # start = time.time()
        # for action in self.actions:
        #     action.window.show_all()
        # end = time.time()
        # print(end - start)

        # time.sleep(1)

        # start = time.time()
        # for action in self.actions:
        #     action.window.hide()
        # end = time.time()
        # print(end - start)

        # time.sleep(1)

        # start = time.time()
        # for action in self.actions:
        #     action.window.show_all()
        # end = time.time()
        # print(end - start)

class Registry:
    def __init__(self, config, updates_callback):
        self.config = config
        self.updates_callback = updates_callback
        self.cached_windows = {}

    def refresh_all(self):
        """Refresh window list."""
        self.cached_windows = {}
        desktop = pyatspi.Registry.getDesktop(0)
        for application in desktop:
            for window in application:
                cached_window = CachedWindow(self.config, window)
                self.cached_windows[cached_window.id] = cached_window
                self.refresh(window)
                # self.cached_windows.append(cached_window)
                # self.updates_callback(cached_window)

    def refresh(self, window):
        if window.accessibleId not in self.cached_windows:
            cached_window = CachedWindow(self.config, window)
            self.cached_windows[cached_window.id] = cached_window

        cached_window = self.cached_windows[window.accessibleId]
        cached_window.clear()
    #     collection = self.window.queryCollection()
    #     rule = collection.createMatchRule(
    #         pyatspi.StateSet.new(self.config.states), collection.MATCH_ALL,
    #         "", collection.MATCH_NONE,
    #         [role for role in self.config.roles], collection.MATCH_ANY,
    #         "", collection.MATCH_NONE,
    #         False)
    #     # TODO: is there a better order?
    #     accessibles = collection.getMatches(rule, collection.SORT_ORDER_CANONICAL, 0, True)
    #     self.actions = []
    #     start = time.time()
    #     for accessible in accessibles:
    #         action_type = self.config.get_action_type(accessible)
    #         if action_type is not None:
    #             self.actions.append(new_action(self.config, accessible, action_type))
    #     end = time.time()
    #     print(end - start)

    def get_actions(self, window):
        """Get actions for the current window."""
        # if self.focused_window < 0:
        #     window = get_focused_window()
        #     if not window:
        #         return []
        #     for index, cached_window in enumerate(self.cached_windows):
        #         if window == cached_window.window:
        #             self.focused_window = index
        #             return cached_window.actions
        #     self.focused_window = len(self.cached_windows)
        #     self.cached_windows.append(_CachedWindow(self.config, window))
        #     return self.cached_windows[self.focused_window].actions
        # elif self.focused_window < len(self.cached_windows):
        #     return self.cached_windows[self.focused_window].actions
        # else:
        #     return []
        for cached_window in self.cached_windows:
            if cached_window.window == window:
                return cached_window.actions

        self.cached_windows.append(CachedWindow(self.config, window))