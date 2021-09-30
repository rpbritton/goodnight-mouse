<!--
 Copyright (C) 2021 Ryan Britton

 This file is part of Goodnight Mouse.

 Goodnight Mouse is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Goodnight Mouse is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Goodnight Mouse.  If not, see <http://www.gnu.org/licenses/>.
-->

# ![Goodnight Mouse](examples/readme/logo.png)

Quickly label all button and links with a short key sequence as an alternative to using the mouse.

## Example Usage
![example-workflow](./examples/readme/example-workflow.gif)

## Features
* Click buttons, follow links, and focus text with the keyboard.
* Hold shift to use the "shifted state" that produces alternative actions, such as closing a tab.
* Update and add new labels when the application changes while GM is open.
* The arrow keys and others are passed through to the application for movement while open.
* Configurable theme via the config file.

## Requirements
* At-Spi2

## Build
This project uses the meson build system.
```
meson setup builddir
cd builddir
meson compile
```

## Run
See the options with `./builddir/goodnight_mouse -h`.
<br>
Check out the sample config in `./examples/config/goodnight_mouse.cfg`
<br>
Try it with `./builddir/goodnight_mouse -c examples/config/goodnight_mouse.cfg`

## Tips for getting accessibility working
* Check out this repository: https://salsa.debian.org/a11y-team/check-a11y
    * Specifically, try running `./troubleshoot`
* Environment variables:
```
export GTK_MODULES=gail:atk-bridge
export OOO_FORCE_DESKTOP=gnome
export GNOME_ACCESSIBILITY=1
export QT_ACCESSIBILITY=1
export QT_LINUX_ACCESSIBILITY_ALWAYS_ON=1
export ACCESSIBILITY_ENABLED=1
```
* Set DBus properties to true:
  * `org.a11y.Status.IsEnabled`
  * `org.a11y.Status.ScreenReaderEnabled`
* Run electron and chrome apps with `--force-renderer-accessibility`
  * If the popups are off it may be caused by scaling, such as `Xft.dpi` in Xresources
* `dbus-monitor` the at-spi to see if things are working, like mouse input.
  * May need to run `dbus-update-activation-environment --systemd DBUS_SESSION_BUS_ADDRESS DISPLAY XAUTHORITY` at startup, such as in xinit.

### Known Working Applications
* Firefox
* Google Chrome
  * Requires `--force-renderer-accessibility`
* Slack
  * Requires `--force-renderer-accessibility`
* MS Teams
  * Requires `--force-renderer-accessibility`
* Discord
  * Requires `--force-renderer-accessibility`
* VLC
* VS Code
  * Requires `--force-renderer-accessibility`
