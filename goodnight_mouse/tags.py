import pyatspi
import random

tag_states = [pyatspi.STATE_VISIBLE, pyatspi.STATE_SHOWING]

class _NotVisible(Exception):
    pass
class _NoAction(Exception):
    pass
class _InvalidPosition(Exception):
    pass

def create_tags(window):
    tags = _create_tag(window)
    _sort_tags(tags)
    _code_tags(tags)
    return tags
    
def _create_tag(accessible):
    tags = []

    try:
        tags.append(Tag(accessible))
    except _NotVisible:
        return []
    except:
        None

    for childAccessible in accessible:
        tags += _create_tag(childAccessible)

    return tags
        
def _sort_tags(tags):
    # TODO: sort by closest to center (of window?)
    # actually probably don't bother with that, sort left to right, top to bottom
    None

def _code_tags(tags):
    # TODO: make better lol
    for index in range(10):
        if index >= len(tags):
            break
        tags[index].code = chr(index+97) + chr(random.randint(97, 97+25))
        if index % 2 == 0:
            tags[index].code += chr(random.randint(97, 97+25))
        if index % 3 == 0:
            tags[index].code += "i"

class Tag:
    def __init__(self, accessible):
        self.accessible = accessible
        self.code = None

        # only bother with the visible
        states = accessible.getState()
        for state in tag_states:
            if not states.contains(state):
                raise _NotVisible

        # make sure it is actionable
        try:
            action = accessible.queryAction()
        except:
            raise _NoAction
        # just checking, although pretty sure the above will error
        if action.get_nActions() < 1:
            raise _NoAction
        self.action = action

        # TODO: this could be a cool feature to run again if a menu item
        # Actually probably need to use roles; imagine button in a button
        # if accessible.getChildCount() > 0:
        #     print("children:", accessible.getChildCount())

        position = accessible.queryComponent().getPosition(0)
        self.x, self.y = position[0], position[1]

        # TODO: figure out those pesky weird positions (e.g. constraint to the window)
        #raise InvalidPosition