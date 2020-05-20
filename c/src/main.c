#include <stdio.h>
#include <X11/Xlib.h>

#include <glib-object.h>
#include <atspi/atspi-accessible.h>
#include <atspi/atspi-collection.h>
#include <atspi/atspi-matchrule.h>

#include "focus.h"

int count = 0;

void recurse(AtspiAccessible *accessible) {
    count++;
    int child_count = atspi_accessible_get_child_count(accessible, NULL);
    for (int child_index = 0; child_index < child_count; child_index++) {
        recurse(atspi_accessible_get_child_at_index(accessible, child_index, NULL));
    }
}

int main() {
    Display *display = XOpenDisplay(NULL);

    AtspiAccessible *window = focus_get_active_window(display);
    if (!window) {
        printf("active window is not accessible\n");
        exit(1);
    }

    // recurse(window);

    // printf("%d\n", count);
    // printf("%s\n", atspi_accessible_get_name(window, NULL));
    // AtspiAccessible *child = atspi_accessible_get_child_at_index(window, 0, NULL);
    // recurse(child);
    // printf("%d\n", count);
    AtspiCollection *collection = atspi_accessible_get_collection_iface(window);
    AtspiStateSet *state_set = atspi_state_set_new(NULL);
    atspi_state_set_add(state_set, ATSPI_STATE_SHOWING);
    atspi_state_set_add(state_set, ATSPI_STATE_VISIBLE);
    atspi_state_set_add(state_set, ATSPI_STATE_SENSITIVE);
    AtspiMatchRule *rule = atspi_match_rule_new(
        state_set, ATSPI_Collection_MATCH_ANY,
        NULL, ATSPI_Collection_MATCH_NONE,
        NULL, ATSPI_Collection_MATCH_NONE,
        NULL, ATSPI_Collection_MATCH_NONE,
        FALSE);
    GArray *results = atspi_collection_get_matches(collection, rule, ATSPI_Collection_SORT_ORDER_CANONICAL, 0, TRUE, NULL);
    printf("%d\n", results->len);
    // printf("%d\n", collection);
    // char *name = atspi_accessible_get_name(window, NULL);
    // int child_count1 = atspi_accessible_get_child_count(window, NULL);
    // int child_count2 = atspi_accessible_get_child_count(window, NULL);
    // int child_count3 = atspi_accessible_get_child_count(window, NULL);
    // int child_count4 = atspi_accessible_get_child_count(window, NULL);

    // printf("active application: %s\n", atspi_accessible_get_name(window, NULL));

    XCloseDisplay(display);

    return 0;
}