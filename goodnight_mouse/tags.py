import gi
gi.require_version("Gtk", "3.0")
gi.require_version("Gdk", "3.0")
gi.require_version("Atspi", "2.0")
from gi.repository import Gtk, Gdk, Atspi

import random

valid_tag_states = [Atspi.StateType.VISIBLE, Atspi.StateType.SHOWING]

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
    except NoAction:
        None

    for child_accessible_index in range(accessible.get_child_count()):
        tags += _create_tags(accessible.get_child_at_index(child_accessible_index))

    return tags
        
def _sort_tags(tags):
    # TODO: sort by closest to center (of window?)
    # actually probably don't bother with that, sort left to right, top to bottom
    None

def _add_keys(tags):
    # TODO: make better lol
    for index in range(len(tags)):
        tags[index].keys = chr(index+97+index%2) + chr(random.randint(97, 97+25))
        if index % 2 == 0:
            tags[index].keys += chr(random.randint(97, 97+25))
        if index % 3 == 0:
            tags[index].keys += "i"

class Tag:
    def __init__(self, accessible):
        self._accessible = accessible
        self.keys = None

        # only bother with the visible
        states = accessible.get_state_set()
        for state in valid_tag_states:
            if not states.contains(state):
                raise NotVisible

        # make sure it is actionable
        self._action = accessible.get_action_iface()
        if self._action == None:
            raise NoAction
        # just checking, although pretty sure the above will error
        if self._action.get_n_actions() < 1:
            raise NoAction
        # TODO: how to handle more than 1 action? (never seen)

        # TODO: this could be a cool feature to run again if a menu item
        # Actually probably need to use roles; imagine button in a button
        # if accessible.getChildCount() > 0:
        #     print("children:", accessible.getChildCount())

        position = accessible.get_component_iface().get_position(Atspi.CoordType.SCREEN)
        self._x, self._y = position.x, position.y

        # TODO: figure out those pesky weird positions (e.g. constrain to the window)
        #raise InvalidPosition

    def _create_window(self):
        # TODO: delete old gui
        if self.keys == None:
            return
        
        self._window = Gtk.Window(type = Gtk.WindowType.POPUP)
        self._window.move(self._x, self._y)
        # self.window.set_decorated(False) # TODO: not needed?
        # TODO: make window semi-transparent?

        # button = Gtk.Button()
        # button.connect("clicked", self._clicked)
        # button.get_style_context().add_class("tag")

        labels = Gtk.Box()
        labels.set_halign(Gtk.Align.CENTER)
        labels.get_style_context().add_class("tag")

        self._key_labels = []
        for key in iter(self.keys):
            key_label = Gtk.Label(key)
            key_label.get_style_context().add_class("key")
            self._key_labels.append(key_label)

            labels.add(key_label)

        # button.add(labels)

        self._window.add(labels)
        self._window.show_all()

    def _clicked(self, event):
        self.run()
    
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
        Gdk.Display.get_default().get_default_seat().ungrab()
        Gtk.main_quit()
        self._action.do_action(0)
        exit()