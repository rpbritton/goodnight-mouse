import pyatspi
import random

tag_states = [pyatspi.STATE_VISIBLE, pyatspi.STATE_SHOWING]

class NotVisible(Exception):
    pass
class NoAction(Exception):
    pass
class InvalidPosition(Exception):
    pass

class Tags:
    def __init__(self, window):
        self.tags = []
        self._create_tags(window)
        self._sort_tags()
        self._code_tags()

    def get_tags(self):
        return self.tags
    
    def _create_tags(self, accessible):
        try:
            self.tags.append(Tag(accessible))
        except NotVisible:
            return
        except:
            None

        for childAccessible in accessible:
            self._create_tags(childAccessible)
        
    def _sort_tags(self):
        # TODO: sort by closest to center (of window?)
        None

    def _code_tags(self):
        # TODO: make better lol
        for index in range(10):
            if index >= len(self.tags):
                break
            self.tags[index].code = chr(index+97) + chr(random.randint(97, 97+25))
            if index % 2 == 0:
                self.tags[index].code += chr(random.randint(97, 97+25))
            if index % 3 == 0:
                self.tags[index].code += "i"

class Tag:
    def __init__(self, accessible):
        self.accessible = accessible
        self.code = None

        # only bother with the visible
        states = accessible.getState()
        for state in tag_states:
            if not states.contains(state):
                raise NotVisible

        # make sure it is actionable
        try:
            action = accessible.queryAction()
        except:
            raise NoAction
        # just checking, although pretty sure the above will error
        if action.get_nActions() < 1:
            raise NoAction
        self.action = action

        # TODO: this could be a cool feature to run again if a menu item
        # Actually probably need to use roles; imagine button in a button
        # if accessible.getChildCount() > 0:
        #     print("children:", accessible.getChildCount())

        position = accessible.queryComponent().getPosition(0)
        self.x, self.y = position[0], position[1]

        # TODO: figure out those pesky weird positions (e.g. constraint to the window)
        #raise InvalidPosition