import argparse
import logging

from goodnight_mouse import app
from goodnight_mouse import debugger

def start():
    parser = argparse.ArgumentParser(description = "Click with your keyboard.")

    parser.add_argument("-d", "--debugger", help = "launch the debugger", action = "store_true")
    parser.add_argument("-v", "--verbose", help = "increase logging", action = "store_true")

    args = parser.parse_args()
    
    if args.debugger:
        debugger.start()
    else:
        app.start()


