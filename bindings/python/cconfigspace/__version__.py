import os


VERSION_FILE = os.path.dirname(os.path.abspath(__file__), "VERSION")

with open(VERSION_FILE, "r") as f:
    __version__ = f.read()
