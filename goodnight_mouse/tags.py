import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
from gi.repository import Gtk, Gdk

import pyatspi
import random

tag_states = [pyatspi.STATE_VISIBLE, pyatspi.STATE_SHOWING]

class NotVisible(Exception):
    pass
class NoAction(Exception):
    pass
# TODO: implement?
class InvalidPosition(Exception):
    pass

def create_tags(window):
    tags = _create_tags(window)
    _sort_tags(tags)
    _add_keys(tags)
    for tag in tags:
        tag._create_window()
    return tags

def _create_tags(accessible):
    tags = []

    try:
        tags.append(Tag(accessible))
    except NotVisible:
        return []
    except:
        None

    for childAccessible in accessible:
        tags += _create_tags(childAccessible)

    return tags
        
def _sort_tags(tags):
    # TODO: sort by closest to center (of window?)
    # actually probably don't bother with that, sort left to right, top to bottom
    None

def _add_keys(tags):
    # TODO: make better lol
    for index in range(len(tags)):
        tags[index].keys = chr(index+97) + chr(random.randint(97, 97+25))
        if index % 2 == 0:
            tags[index].keys += chr(random.randint(97, 97+25))
        if index % 3 == 0:
            tags[index].keys += "i"

class Tag:
    def __init__(self, accessible):
        self._accessible = accessible
        self.keys = None

        # only bother with the visible
        states = accessible.getState()
        for state in tag_states:
            if not states.contains(state):
                raise NotVisible

        # make sure it is actionable
        try:
            self._action = accessible.queryAction()
        except:
            raise NoAction
        # just checking, although pretty sure the above will error
        if self._action.get_nActions() < 1:
            raise NoAction

        # TODO: this could be a cool feature to run again if a menu item
        # Actually probably need to use roles; imagine button in a button
        # if accessible.getChildCount() > 0:
        #     print("children:", accessible.getChildCount())

        position = accessible.queryComponent().getPosition(0)
        self._x, self._y = position[0], position[1]

        # TODO: figure out those pesky weird positions (e.g. constrain to the window)
        #raise InvalidPosition

    def _create_window(self):
        # TODO: delete old gui
        if self.keys == None:
            return
        
        self._window = Gtk.Window(type = Gtk.WindowType.POPUP)
        self._window.move(self._x, self._y)
        # self.window.set_decorated(False) # TODO: not needed?
        # TODO: make window part transparent?

        button = Gtk.Button()
        button.get_style_context().add_class("tag")

        labels = Gtk.Box()
        labels.set_halign(Gtk.Align.CENTER)

        self._key_labels = []
        for key in iter(self.keys):
            key_label = Gtk.Label(key)
            key_label.get_style_context().add_class("key")
            self._key_labels.append(key_label)

            labels.add(key_label)

        button.add(labels)

        self._window.add(button)
        self._window.show_all()
    
    def update(self, keys):
        if keys == self.keys:
            self.run()
        elif self.keys.startswith(keys):
            for index in range(len(self.keys)):
                style_context = self._key_labels[index].get_style_context()
                if index < len(keys):
                    style_context.add_class("key_complete")
                else:
                    style_context.remove_class("key_complete")

            self._window.show()
        else:
            self._window.hide()

    def run(self):
        print("done")
        exit()