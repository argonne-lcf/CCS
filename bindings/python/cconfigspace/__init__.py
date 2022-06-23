import platform
import ctypes as ct
import os

if os.environ.get('LIBCCONFIGSPACE_SO_'):
  libcconfigspace = ct.cdll.LoadLibrary(os.environ.get('LIBCCONFIGSPACE_SO_'))
else:
  if platform.uname()[0] == "Linux":
    libcconfigspace = ct.cdll.LoadLibrary('libcconfigspace.so.0.0.0')
  else:
    libcconfigspace = ct.cdll.LoadLibrary('libcconfigspace.dylib')

from .base import *
from .error_stack import *
from .rng import *
from .interval import *
from .distribution import *
from .hyperparameter import *
from .expression import *
from .expression_parser import *
from .context import *
from .configuration_space import *
from .binding import *
from .configuration import *
from .features_space import *
from .features import *
from .objective_space import *
from .evaluation import *
from .features_evaluation import *
from .tuner import *
from .features_tuner import *
