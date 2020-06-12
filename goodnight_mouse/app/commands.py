from typing import Set

from .app import App
from .config import Config


class Commands:
    def __init__(self, config: Config):
        self._config = config

    def start(self):
        with App.new(self._config) as app:
            app.start()

    def trigger(self, flags: Set[str]):
        with App.new(self._config) as app:
            app.trigger(flags)
