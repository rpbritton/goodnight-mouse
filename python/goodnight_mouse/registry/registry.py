import zc.lockfile

def new(config):
    None

class RegistryConnection:
    def __init__(self, config):
        self.config = config

    def trigger(self):
        None

class Registry(RegistryConnection):
    def __init__(self, config):
        self.config = config

    def trigger(self):
        None

    def start(self):
        None