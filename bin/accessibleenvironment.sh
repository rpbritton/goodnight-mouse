#!/bin/echo Please source this file

# Copyright (C) 2021 Ryan Britton
#
# This file is part of Goodnight Mouse.
#
# Goodnight Mouse is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Goodnight Mouse is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Goodnight Mouse.  If not, see <http://www.gnu.org/licenses/>.

case "$1" in
    -s|--set)
        export GTK_MODULES=gail:atk-bridge
        export OOO_FORCE_DESKTOP=gnome
        export GNOME_ACCESSIBILITY=1
        export QT_ACCESSIBILITY=1
        export QT_LINUX_ACCESSIBILITY_ALWAYS_ON=1
        export ACCESSIBILITY_ENABLED=1
    ;;
    -u|--unset)
        unset GTK_MODULES
        unset OOO_FORCE_DESKTOP
        unset GNOME_ACCESSIBILITY
        unset QT_ACCESSIBILITY
        unset QT_LINUX_ACCESSIBILITY_ALWAYS_ON
        unset ACCESSIBILITY_ENABLED
    ;;
    *)
        echo "\
Usage:
  . accessibleenvironment [--set|--unset]

Options:
  -s, --set
  -u, --unset
    " ;;
esac
