import pyatspi
from Xlib.keysymdef import miscellany as keysym

class KeysHandler:
    def __init__(self, action_list):
        self._keys = ""
        self._action_list = action_list
        
        pyatspi.Registry().registerKeystrokeListener(self.handle, mask=range(0xFF))

    def apply(self, keys):
        if keys == self._keys:
            return

        if self._action_list.valid(keys):
            self._keys = keys
        else:
            self._keys = ""
        
        self._action_list.apply(self._keys)

        if self._action_list.match(self._keys):
            pyatspi.Registry().stop()

    def handle(self, event):
        if event.id == keysym.XK_Escape:
            exit()

        if event.id == keysym.XK_BackSpace:
            if event.type == pyatspi.deviceevent.KEY_PRESSED_EVENT:
                self.apply(self._keys[:-1])
        elif 0x00 <= event.id <= 0xFF:
            if event.type == pyatspi.deviceevent.KEY_PRESSED_EVENT:
                self.apply(self._keys + chr(event.id))
        else:
            return False

        return True