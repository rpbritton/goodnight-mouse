import pyatspi
import time
import asyncio

def handler(event):
    # print("hiya", event)
    # print(event.source.getState().getStates())
    # print("id:", event.source.name)

    # print("event type", event)
    try:
        start = time.time()
        # collection = self.window.queryCollection()
        collection = event.source.queryCollection()
        rule = collection.createMatchRule(
            # pyatspi.StateSet.new([pyatspi.STATE_SHOWING, pyatspi.STATE_VISIBLE, pyatspi.STATE_SENSITIVE]), collection.MATCH_ALL,
            pyatspi.StateSet.new([]), collection.MATCH_NONE,
            "", collection.MATCH_NONE,
            # [pyatspi.ROLE_LINK, pyatspi.ROLE_PUSH_BUTTON], collection.MATCH_ANY,
            [], collection.MATCH_NONE,
            "", collection.MATCH_NONE,
            False)
        # TODO: is there a better order?
        accessibles = collection.getMatches(rule, collection.SORT_ORDER_CANONICAL, 0, True)
        end = time.time()
        if len(accessibles) > 50:
            print(end - start, "for", len(accessibles), "from", event.type, event.source)
    except:
        None
        # print(event.type)
    # print(accessibles)
    # print([[accessible.name, accessible.accessibleId] for accessible in accessibles])
    # print(any([accessible == event.source for accessible in accessibles]))
    # for ac
    # lowest = 0
    # highest = 9999999999999
    # for accessible in accessibles:
    #     component = accessible.queryComponent()
    #     x, y = component.getPosition(pyatspi.component.XY_SCREEN)
    #     if y > lowest:
    #         lowest = y
    #     if y < highest:
    #         highest = y
    # print("lowest:", lowest, "highest:", highest)

def threadfunction(accessible_chunk):
    for accessible in accessible_chunk:
        print("start")
        # wasteStart = time.time()
        accessibleStates = accessible.getState().getStates()
        # wasteEnd = time.time()
        # totalWaste += wasteEnd - wasteStart
        total = 0
        if set([pyatspi.STATE_SHOWING, pyatspi.STATE_VISIBLE, pyatspi.STATE_SENSITIVE]).issubset(set(accessibleStates)):
            # total += 1
            total = 1
        print("done?")

def recurse(accessible):
    # collection = self.window.queryCollection()
    collection = accessible.queryCollection()
    rule = collection.createMatchRule(
        pyatspi.StateSet.new([pyatspi.STATE_SHOWING, pyatspi.STATE_VISIBLE, pyatspi.STATE_SENSITIVE]), collection.MATCH_ALL,
        # pyatspi.StateSet.new([]), collection.MATCH_NONE,
        "", collection.MATCH_NONE,
        # [pyatspi.ROLE_DOCUMENT_WEB], collection.MATCH_ANY,
        [], collection.MATCH_NONE,
        "", collection.MATCH_NONE,
        False)
    # TODO: is there a better order?
    accessibles = collection.getMatches(rule, collection.SORT_ORDER_CANONICAL, 0, False)
    result = accessibles[:]
    for child in accessibles:
        result += recurse(child)
    return result

idiottime = 0

def recurse2(accessible):
    global idiottime
    result = []
    start = time.time()
    states = accessible.getState().getStates()
    value = set([pyatspi.STATE_SHOWING, pyatspi.STATE_VISIBLE, pyatspi.STATE_SENSITIVE]).issubset(set(states))
    end = time.time()
    idiottime += end-start
    if value:
        result = [accessible]
        for child in accessible:
            result += recurse2(child)
    return result

for application in pyatspi.Registry.getDesktop(0):
    try:
        # if application.name == "Firefox" or application.name == "moserial":
        start = time.time()
        # collection = self.window.queryCollection()
        collection = application.queryCollection()
        rule = collection.createMatchRule(
            pyatspi.StateSet.new([pyatspi.STATE_SHOWING, pyatspi.STATE_VISIBLE, pyatspi.STATE_SENSITIVE]), collection.MATCH_ALL,
            # pyatspi.StateSet.new([]), collection.MATCH_NONE,
            "", collection.MATCH_NONE,
            # [pyatspi.ROLE_LINK, pyatspi.ROLE_PUSH_BUTTON], collection.MATCH_ANY,
            [], collection.MATCH_NONE,
            "", collection.MATCH_NONE,
            False)
        # TODO: is there a better order?
        accessibles = collection.getMatches(rule, collection.SORT_ORDER_CANONICAL, 0, True)

        end = time.time()
        print(application.name, end - start, "for", len(accessibles))
    except:
        print(application.name)

def main():
    for application in pyatspi.Registry.getDesktop(0):
        if application.name == "Firefox":
            start = time.time()
            newaccessibles = recurse(application)
            end = time.time()
            print(application.name, end - start, "for", len(newaccessibles))

            start = time.time()
            newaccessibles2 = []
            for window in application:
                newaccessibles2 += recurse2(window)
            end = time.time()
            print(application.name, end - start, "for", len(newaccessibles2), "waste", idiottime)

if __name__ == "__main__":
    main()
    # asyncio.run(main())