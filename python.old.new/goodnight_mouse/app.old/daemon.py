import pyatspi
import time
# import gi
# gi.require_version("Gtk", "3.0")
# gi.require_version("Gdk", "3.0")
# from gi.repository import Gtk, Gdk

# from .css import css

# # from .config import Config
# from .focus import FocusHandler
# from .action_list import ActionList
# from .keys import KeysHandler
# from .mouse import MouseHandler

def main(config):
    while True:
        print("yay")
        time.sleep(5)
# def recurse(accessible):
#     count = 1
#     for child in accessible:
#         count += recurse(child)

#     return count

# def main(raw_config):
#     # times = []
#     # times.append(time.time())
#     # css_provider = Gtk.CssProvider()
#     # css_provider.load_from_data(css)
#     # Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

#     count = 0
#     while count < 10:
#         focus_handler = FocusHandler()
#         window = focus_handler.get_window()
#         print(window.name)
#         collection = window.queryCollection()
#         rule = collection.createMatchRule(
#             pyatspi.StateSet.new([pyatspi.STATE_SHOWING, pyatspi.STATE_VISIBLE, pyatspi.STATE_ENABLED, pyatspi.STATE_SENSITIVE]), collection.MATCH_ALL,
#             "", collection.MATCH_NONE,
#             [], collection.MATCH_NONE,
#             "", collection.MATCH_NONE,
#             False)
#         accessibles = collection.getMatches(rule, collection.SORT_ORDER_CANONICAL, 0, True)
#         print(window.name, len(accessibles))
#         count+=1
#     # print(recurse(focus_handler.get_window()))
#     # print(window.clear_cache())
#     # print(recurse(focus_handler.get_window()))
#     # print(recurse(focus_handler.get_window()))
#     # # TODO: is this the right place
#     # window = focus_handler.get_window()
#     # if window == None:
#     #     exit()
#     # times.append(time.time())

#     # config = Config(raw_config, window)
#     # times.append(time.time())

#     # action_list = ActionList(config, focus_handler.get_window())
#     # times.append(time.time())

#     # keys_handler = KeysHandler(action_list)
#     # times.append(time.time())
#     # mouse_handler = MouseHandler()
#     # times.append(time.time())

#     # for index in range(len(times) - 1):
#     #     print("app: index", index, "time", times[index+1] - times[index])

#     # pyatspi.Registry.start()

#     # action_list.do()