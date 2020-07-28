import ctypes as ct
import os

if os.environ.get('LIBCCONFIGSPACE_SO_'):
  libcconfigspace = ct.cdll.LoadLibrary(os.environ.get('LIBCCONFIGSPACE_SO_'))
else:
  libcconfigspace = ct.cdll.LoadLibrary('libcconfigspace.so.0.0.0')

from .base import *
from .rng import *
