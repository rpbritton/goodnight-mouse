import time

import pyatspi
from Xlib import X, display, ext
from gi.repository import GLib


class Emulation:
    dis = display.Display()
    root = dis.screen().root

    @classmethod
    def mouse_tap(cls, button: int, x: int, y: int):
        pointer = cls.root.query_pointer()
        cls.mouse_move(x, y)
        cls.mouse_press(button)
        cls.mouse_release(button)
        cls.mouse_move(pointer.root_x, pointer.root_y)

    @classmethod
    def mouse_move(cls, x: int, y: int):
        cls.root.warp_pointer(x, y)
        cls.wait()

    @classmethod
    def mouse_press(cls, button: int):
        ext.xtest.fake_input(cls.dis, X.ButtonPress, button)
        cls.wait()

    @classmethod
    def mouse_release(cls, button: int):
        ext.xtest.fake_input(cls.dis, X.ButtonRelease, button)
        cls.wait()

    @classmethod
    def key_tap(cls, key: int):
        cls.key_press(key)
        cls.key_release(key)

    @classmethod
    def key_press(cls, key: int):
        ext.xtest.fake_input(cls.dis, X.KeyPress, key)
        cls.wait()

    @classmethod
    def key_release(cls, key: int):
        ext.xtest.fake_input(cls.dis, X.KeyRelease, key)
        cls.wait()

    @classmethod
    def wait(cls):
        cls.dis.sync()
        time.sleep(0.001)


class ImmediateTimeout:
    enabled = False
    _log_handler_id = None

    @classmethod
    def enable(cls):
        if cls.enabled:
            return
        cls.enabled = True

        # hide the warnings that result from immediately timing out
        cls._log_handler_id = GLib.log_set_handler(
            "dbind", GLib.LogLevelFlags.LEVEL_WARNING, lambda *args: None, None)

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

    @classmethod
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
