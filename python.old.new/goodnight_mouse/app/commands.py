from .app import new_app
import time

class Commands:
    def __init__(self, config):
        self.config = config
        self.app = new_app(config)

    def start(self):
        self.app.start_background()

    def trigger(self):
        self.app.trigger()