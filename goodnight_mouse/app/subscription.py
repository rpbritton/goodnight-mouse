class Subscription:
    def __init__(self):
        self._subscribers = set()

    def subscribe(self, subscriber):
        if subscriber not in self._subscribers:
            self._subscribers.add(subscriber)

    def unsubscribe(self, subscriber):
        if subscriber in self._subscribers:
            self._subscribers.remove(subscriber)

    def notify(self, *args):
        consume = False

        for subscriber in self._subscribers:
            if subscriber(*args) is True:
                consume = True

        return consume
