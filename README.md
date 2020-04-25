# goodnight-mouse

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
* Run electron and chrome apps with `--force-renderer-accessibility`

## Run

`python -m goodnight_mouse`