import logging

import pyatspi

from .focus import Focus


class Background:
    def __init__(self, focus: Focus):
        self._focus = focus

    def __enter__(self):
        self._focus.subscribe(self._focus_handle)

        return self

    def __exit__(self, *args):
        self._focus.unsubscribe(self._focus_handle)

    def _focus_handle(self, window: pyatspi.Accessible):
        if window is not None:
            # make sure caching/connection occurs
            window_name = window.name
            application_name = window.getApplication().name
            logging.debug("focuse changed to window '%s' from application '%s'",
                          window_name, application_name)
        else:
            logging.debug("focuse changed to none")
