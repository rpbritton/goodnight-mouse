class Controller:
    def __init__(self):
        self.running = False

    def start(self):
        if self.running:
            return False
        else:
            self.running = True
            return True

    def stop(self):
        if not self.running:
            return False
        else:
            self.running = False
            return True

    def is_running(self):
        return self.running
