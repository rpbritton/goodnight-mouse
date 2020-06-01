import argparse
import yaml

from .app import Commands
# from . import debugger

def main():
    parser = argparse.ArgumentParser(
        prog="goodnight_mouse",
        description="Click with your keyboard.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

    parser.add_argument("command", help="program to run",
                        choices=["debugger", "background", "trigger"],
                        nargs="?", default="trigger")
    parser.add_argument("-v", "--verbose", help="increase logging", action="store_true")
    parser.add_argument("-c", "--config", help="config file to read", default="~/.config/goodnight_mouse/config.yaml")

    args = parser.parse_args()

    config = yaml.safe_load(open(args.config))

    if args.command == "background":
        commands = Commands(config)
        commands.start()
    elif args.command == "trigger":
        commands = Commands(config)
        commands.trigger()
    elif args.command in ("debugger"):
        # debugger.main(config)
        None