import pyatspi
from gi.repository import GLib


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
