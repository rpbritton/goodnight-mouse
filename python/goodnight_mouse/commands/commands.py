from .. import registry

class Commands:
    def __init__(self, config):
        self.config = config
        self.registry = registry.new(config)

    def start(self):
        self.registry.start()

    def trigger(self):
        self.registry.trigger()