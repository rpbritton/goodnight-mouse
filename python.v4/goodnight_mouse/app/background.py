import pyatspi

from .focus import Focus

class Background:
    def __init__(self, focus: Focus):
        self._focus = focus

    def __enter__(self):
        self._focus.subscribe(self._focus_handle)

    def __exit__(self, type, value, traceback):
        self._focus.unsubscribe(self._focus_handle)

    def _focus_handle(self, window: pyatspi.Accessible):
        print("focused:", window)
        if window is not None:
            # make sure caching/connection occurs
            window.name
            window.getApplication().name