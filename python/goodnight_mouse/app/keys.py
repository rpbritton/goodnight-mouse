import pyatspi
from gi.repository import GLib

class KeysHandler:
    def __init__(self, callback):
        self.callback = callback

    def start(self):
        # prevent key events from causing blocking time out issues
        # normally need to wait 3 seconds if there's a waiting key event,
        # but they seem to go through
        pyatspi.setTimeout(0, 0)
        # the above function doesn't actually force a refresh on the time out
        pyatspi.Registry.getDesktop(0).name

        # the above causes some nasty warnings when it's time to register
        log_handler_id = GLib.log_set_handler("dbind", GLib.LogLevelFlags.LEVEL_WARNING, lambda *args: None, None)

        # register keystrokes with every possible comination of modifiers
        pyatspi.Registry.registerKeystrokeListener(
            self.handle,
            mask=pyatspi.allModifiers(),
            synchronous=False)

        # remove the logging handler
        GLib.log_remove_handler("dbind", log_handler_id)

        # reset timeout to default settings
        pyatspi.setTimeout(800, 15000)
        # again, probably want to "flush" the timeout
        pyatspi.Registry.getDesktop(0).name

    def stop(self):
        pyatspi.Registry.deregisterKeystrokeListener(
            self.handle,
            mask=pyatspi.allModifiers())

    def handle(self, event):
        if event.type == pyatspi.deviceevent.KEY_PRESSED_EVENT:
            self.callback(event.id)
        return True