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

## Run
```python -m goodnight_mouse --help```

## FAQ

### Why does this use its own hotkey listener?

It is generally not possible in X11 for a hotkey manager to listen
to for keys in popup menus, since the popup menu has a global grab.
Furthermore temporary menus (like the autocompletion suggestions 
in a web browser), I have found to go away. GM works by using the
applications accessibility toolkit to intercept the keys.
