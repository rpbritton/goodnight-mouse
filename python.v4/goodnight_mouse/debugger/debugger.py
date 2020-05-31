import atexit

import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
gi.require_version("Atspi", "2.0")
from gi.repository import Atspi, Gdk, Gtk


def main(config):
    Atspi.init()
    atexit.register(exit_callback)

    key_listener = Atspi.DeviceListener.new(handle_key)
    Atspi.register_keystroke_listener(
        key_listener, None, 0, 0, Atspi.KeyListenerSyncType.CANCONSUME)

    mouse_listener = Atspi.EventListener.new(handle_mouse)
    mouse_listener.register("mouse")

    Atspi.event_main()


def exit_callback():
    Atspi.exit()


def handle_mouse(event):
    print(event.type)
    print(event.sender, event.source)
    # Atspi.event_quit()


def handle_key(event):
    if event.type == Atspi.EventType.KEY_PRESSED_EVENT and event.id == Gdk.KEY_Escape:
        exit()
