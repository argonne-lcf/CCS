import ctypes as ct
import os
import platform
import packaging.version
import warnings

from ctypes.util import find_library

# from .__version__ import __version__
__version__ = "0.0.1"


__ccs_versions___ = [
  packaging.version.parse(__version__), 
  packaging.version.parse(__version__)
  ] # [min, max] ccs versions



# load configspace library

# test input library from the user
libcconfigspace_file = os.environ.get("LIBCCONFIGSPACE_SO_")

if libcconfigspace_file is not  None:
  # try loading the user specified library
  libcconfigspace = ct.cdll.LoadLibrary(libcconfigspace_file)
else:
  # test if system library is available (or if the library ws already loaded)
  libcconfigspace_file = find_library("libcconfigspace")

  # default to package lib
  if libcconfigspace_file is None:

    # look for the correct lib extension for the current platform
    system = platform.uname()[0]
    if system == "Linux":
      lib_ext = "so.0.0.0"
    elif system == "Darwin":  # MacOS
      lib_ext = "0.dylib"
    else: # Windows
      lib_ext = "dll"

    # create a versioned name for the library (is the library already openened?)
    libcconfigspace_file = f"libcconfigspace.{lib_ext}"

  # try loading the system library
  try:
    libcconfigspace = ct.cdll.LoadLibrary(libcconfigspace_file)
  except OSError:
    # set of path to retrieve build directory
    package_dir = os.path.dirname(os.path.abspath(__file__))
    libcconfigspace_file = os.path.join(package_dir, f"libcconfigspace.{lib_ext}")
    libcconfigspace = ct.cdll.LoadLibrary(libcconfigspace_file)

# check if the ccs library is allowed
from .base import ccs_get_version

libcconfigspace_version = packaging.version.parse(ccs_get_version().short) 
if __ccs_versions___[0] > libcconfigspace_version or libcconfigspace_version > __ccs_versions___[1]:
  warnings.warn(f"The loaded C-library of CConfigSpace has version {libcconfigspace_version} when it should be >={__ccs_versions___[0]}, <{__ccs_versions___[1]}")

from .base import *
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
