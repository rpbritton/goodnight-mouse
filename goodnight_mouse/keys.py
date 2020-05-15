import gi
gi.require_version("Atspi", "2.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Atspi, Gdk

class KeysHandler:
    def __init__(self, actions):
        self._keys = ""
        self._actions = actions
        
        self._listener = Atspi.DeviceListener.new(self._handle)
        for mod_mask in range(255):
            # TODO: check return type?
            Atspi.register_keystroke_listener(self._listener, None, mod_mask, 0, Atspi.KeyListenerSyncType.CANCONSUME)

    def _handle(self, event):
        # TODO: don't return true necessairly
        if event.type != Atspi.EventType.KEY_PRESSED_EVENT:
            return True

        if event.id == Gdk.KEY_Escape:
            exit()
        elif event.id == Gdk.KEY_BackSpace:
            self._keys = self._keys[:-1]
        elif event.id >= 0 and event.id <= 255:
            self._keys += chr(event.id)
        # TODO:
        # else:
        #     return False

        if not self._actions.valid(self._keys):
            self._keys = ""
        
        self._actions.process(self._keys)

        if self._actions.match(self._keys):
            Atspi.event_quit()

        return True