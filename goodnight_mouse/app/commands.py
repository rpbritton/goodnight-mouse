from .app import App
from .config import Config


class Commands:
    def __init__(self, config: Config):
        self._config = config

    def start(self):
        with App.new(self._config) as app:
            app.background()

    def trigger(self):
        with App.new(self._config) as app:
            app.foreground()
