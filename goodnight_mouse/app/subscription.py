class Subscription:
    def __init__(self):
        self._subscribers = set()

    def subscribe(self, subscriber):
        if subscriber not in self._subscribers:
            self._subscribers.add(subscriber)
            # if len(self._subscribers) == 1:
            #     self.__enter__()

    def unsubscribe(self, subscriber):
        if subscriber in self._subscribers:
            self._subscribers.remove(subscriber)
            # # TODO: does this not immediately remove?
            # if len(self._subscribers) == 0:
            #     self.__exit__()

    def notify(self, *args):
        for subscriber in self._subscribers:
            subscriber(*args)

    # def __enter__(self):
    #     return self

    # def __exit__(self, *args):
    #     pass
