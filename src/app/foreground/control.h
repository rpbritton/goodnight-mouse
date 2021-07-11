/**
 * Copyright (C) 2021 Ryan Britton
 *
 * This file is part of Goodnight Mouse.
 *
 * Goodnight Mouse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Goodnight Mouse is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Goodnight Mouse.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef E6235E1E_3D35_4FBB_8900_4B67D274702D
#define E6235E1E_3D35_4FBB_8900_4B67D274702D

#include <atspi/atspi.h>
#include <gtk/gtk.h>

#define CONTROL_CSS_CLASS_TAG "control_tag"
#define CONTROL_CSS_CLASS_LABEL "control_label"
#define CONTROL_CSS_CLASS_LABEL_ACTIVE "control_label_active"

typedef enum ControlType
{
    CONTROL_TYPE_NONE,
    CONTROL_TYPE_PRESS,
    CONTROL_TYPE_FOCUS,
} ControlType;

typedef struct Control
{
    ControlType type;
    AtspiAccessible *accessible;

    GArray *code;

    GtkFixed *parent;
    GtkWidget *container;
    GtkWidget *tag;
    GArray *labels;
} Control;

Control *control_new(ControlType type, AtspiAccessible *accessible);
void control_destroy(Control *control);
void control_execute(Control *control);
void control_label(Control *control, GArray *code);
void control_unlabel(Control *control);
gboolean control_match(Control *control, GArray *code);
void control_show(Control *control, GtkFixed *container);
void control_hide(Control *control);
void control_reposition(Control *control);

#endif /* E6235E1E_3D35_4FBB_8900_4B67D274702D */
