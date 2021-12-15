import os
import platform
import shutil

from setuptools import Distribution, setup

here = os.path.dirname(os.path.abspath(__file__))

NAME = "cconfigspace"
REQUIRES_PYTHON = ">=3.6"

# Load the package's __version__.py module as a dictionary.
VERSION_IN = os.path.join(os.path.dirname(os.path.dirname(here)), "VERSION")
VERSION_OUT = os.path.join(here, NAME, "VERSION")
shutil.copy(VERSION_IN, VERSION_OUT)

with open(os.path.join(here, NAME, "VERSION")) as f:
  VERSION = f.read()


def get_lib_extension():
	system = platform.uname()[0]
	if system == "Linux":
		lib_ext = "so.0.0.0"
	elif system == "Darwin":  # MacOS
		lib_ext = "0.dylib"
	else:  # Windows
		lib_ext = "dll"
	return lib_ext


REQUIRED = [
	"parglare==0.12.0",
	"packaging"
	]


class BinaryDistribution(Distribution):
	"""Distribution which always forces a binary package with platform name"""

	def has_ext_modules(foo):
		return True


ext_lib = f"libcconfigspace.{get_lib_extension()}"
print(f" *** {ext_lib}")
setup(
	name=NAME,
	version=VERSION,
	author="Brice Videau",
	author_email="bvideau@anl.gov",
	python_requires=REQUIRES_PYTHON,
	url="https://github.com/argonne-lcf/CCS",
	packages=[NAME],
	include_package_data=True,
	package_dir={NAME: NAME},
	package_data={NAME: [
		"VERSION",
		f"*.{get_lib_extension()}"
		]},
	install_requires=REQUIRED,
	license="BSD-3-Clause",
	distclass=BinaryDistribution,
)
