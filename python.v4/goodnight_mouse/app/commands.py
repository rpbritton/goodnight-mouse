from .app import new_app
from .config import Config

class Commands:
    def __init__(self, config: Config):
        self.config = config
        self.app = new_app(config)

    def start(self):
        self.app.background()

    def trigger(self):
        self.app.foreground()