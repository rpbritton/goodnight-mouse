import argparse
import yaml
import logging

from . import app
from . import debugger

def main():
    parser = argparse.ArgumentParser(
        description="Click with your keyboard.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument("-d", "--debugger", help="launch the debugger", action="store_true")
    parser.add_argument("-v", "--verbose", help="increase logging", action="store_true")
    parser.add_argument("-c", "--config", help="config file to read", default="~/.config/goodnight_mouse/config")

    args = parser.parse_args()

    config = yaml.safe_load(open(args.config))

    if args.debugger:
        debugger.main(config)
    else:
        app.main(config)

if __name__ == "__main__":
    main()