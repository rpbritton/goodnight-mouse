# goodnight-mouse

Upon hotkey press, labels all available actions in a program with a short sequence of keys to perform it.

## Requirements
* AT-SPI2

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
        * This should really be done by the script.
* Run electron and chrome apps with `--force-renderer-accessibility`
    * If the popups are off it may be caused by scaling, such as `Xft.dpi` in Xresources
* `dbus-monitor` the at-spi to see if things are working, like mouse input.
    * May need `dbus-update-activation-environment --systemd DBUS_SESSION_BUS_ADDRESS DISPLAY XAUTHORITY` in xinit.

## Other tips
* For picom shadows to work with rounded edges add `full-shadow = true` to `popup_menu` in the `wintypes`

## Run
