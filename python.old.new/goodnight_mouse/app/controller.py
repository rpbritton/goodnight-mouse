class Controller:
    def __init__(self, config, actions):
        self.config = config
        self.actions = actions

    def end(self):
        """Finish and clean up the controller."""
        None