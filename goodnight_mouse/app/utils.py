import time

import pyatspi
from Xlib.display import Display
from gi.repository import GLib

EMPTY_COLLECTION = pyatspi.Collection(None)


class Emulation:
    display = Display()
    root = display.screen().root

    mouse_actions = {
        "click": "b{}c",
        "press": "b{}p",
        "release": "b{}r",
    }

    @classmethod
    def mouse(cls, button: int, action: str, x: int, y: int, modifiers: int, enable_timeout: bool = True):
        if action not in cls.mouse_actions:
            return

        pointer = cls.root.query_pointer()
        if enable_timeout:
            ImmediateTimeout.enable()
        cls.modifiers(modifiers, True, False)
        pyatspi.Registry.generateMouseEvent(
            x, y, cls.mouse_actions[action].format(button))
        cls.modifiers(modifiers, False, False)
        pyatspi.Registry.generateMouseEvent(
            pointer.root_x, pointer.root_y, "abs")
        if enable_timeout:
            ImmediateTimeout.disable()

    key_actions = {
        "click": pyatspi.KEY_PRESSRELEASE,
        "press": pyatspi.KEY_PRESS,
        "release": pyatspi.KEY_RELEASE,
    }

    @classmethod
    def key(cls, key: int, action: str, modifiers: int, enable_timeout: bool = True):
        if action not in cls.key_actions:
            return

        if enable_timeout:
            ImmediateTimeout.enable()
        cls.modifiers(modifiers, True, False)
        pyatspi.Registry.generateKeyboardEvent(
            cls.display.keysym_to_keycode(key), None, cls.key_actions[action])
        cls.modifiers(modifiers, False, False)
        if enable_timeout:
            ImmediateTimeout.disable()

    @classmethod
    def modifiers(cls, modifiers: int, lock: bool, enable_timeout: bool = True):
        if lock:
            lock_type = pyatspi.KEY_LOCKMODIFIERS
        else:
            lock_type = pyatspi.KEY_UNLOCKMODIFIERS

        if enable_timeout:
            ImmediateTimeout.enable()
        pyatspi.Registry.generateKeyboardEvent(
            modifiers, None, lock_type)
        if enable_timeout:
            ImmediateTimeout.disable()


class ImmediateTimeout:
    enabled = False
    _log_handler_id = None

    @ classmethod
    def enable(cls):
        if cls.enabled:
            return
        cls.enabled = True

        # hide the warnings that result from immediately timing out
        cls._log_handler_id = GLib.log_set_handler(
            "dbind", GLib.LogLevelFlags.LEVEL_WARNING, lambda *args: None, None)

        # make sure we get the desktop before setting the instant timeout
        desktop = pyatspi.Registry.getDesktop(0)

        # prevent key events from causing blocking time out issues
        # normally need to wait 3 seconds if there's a waiting key event,
        # but they seem to go through.
        # it's also faster.
        pyatspi.setTimeout(0, 0)

        # force the timeout to actually refresh through a dbus call
        try:
            desktop.accessibleId
        except:
            pass

    @ classmethod
    def disable(cls):
        if not cls.enabled:
            return
        cls.enabled = False

        # reset timeout to default settings
        pyatspi.setTimeout(800, 15000)

        # flush the timeout
        try:
            pyatspi.Registry.getDesktop(0).accessibleId
        except:
            pass

        # remove the logging handler
        GLib.log_remove_handler("dbind", cls._log_handler_id)
        cls._log_handler_id = None
