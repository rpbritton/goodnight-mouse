class Subscription:
    def __init__(self):
        self._subscribers = set()

    def subscribe(self, subscriber):
        if subscriber not in self._subscribers:
            self._subscribers.add(subscriber)
            if len(self._subscribers) == 1:
                self._register()

    def unsubscribe(self, subscriber):
        if subscriber in self._subscribers:
            self._subscribers.remove(subscriber)
            if len(self._subscribers) == 0:
                self._deregister()

    def notify(self, *args):
        for subscriber in self._subscribers:
            subscriber(*args)

    def _register(self):
        pass

    def _deregister(self):
        pass