import platform

import invoke


def get_lib_extension():
	system = platform.uname()[0]
	if system == "Linux":
		lib_ext = "so.0.0.0"
	elif system == "Darwin":  # MacOS
		lib_ext = "0.dylib"
	else:  # Windows
		lib_ext = "dll"
	return lib_ext



def clean_func(c):
	targets = [
		"build", 
		"dist", 
		"**/*.pyc", 
		"cconfigspace/*.dylib",
		"cconfigspace/*.dll", 
		"cconfigspace/*.so*",
		"cconfigspace.egg-info"
		]

	for target in targets:
		c.run("rm -rf {}".format(target))


@invoke.task
def clean(c):
	print("[*] Cleaning starts")
	clean_func(c)
	print("[*] Cleaning completed")


@invoke.task()
def build_ccs(c):
	"""Build the shared library for the sample C code"""
	# Moving this type hint into signature causes an error (???)
	c: invoke.Context

	print("[*] CConfigSpace C-library build starts ")

	with c.cd("../../"):
		c.run("./autogen.sh")

	c.run("mkdir -p build")
	with c.cd("build/"):
		c.run("../../../configure --prefix `pwd`/install")
		c.run("make install")

	print("[*] CConfigSpace C-library build completed")


@invoke.task
def build(c, wheel=False, install=False, dev=False, clean=False):
	"""Build the Python package, automatically trigers the building of the C-CSS library.

	``invoke build --wheel``: build the platform specific wheel without installing.
	``invoke build --install --dev``: build and install the Python package in editable mode for developers.
	``invoke build --install --wheel``: build and install the wheel.

	Args:
			c ([type]): [description]
			wheel (bool, optional): [description]. Defaults to False.
			install (bool, optional): [description]. Defaults to False.
			dev (bool, optional): [description]. Defaults to False.
			clean (bool, optional): [description]. Defaults to False.
	"""

	if clean:
		clean_func(c)
	
	build_ccs(c)

	if wheel or install:

		lib_file = f"libcconfigspace.{get_lib_extension()}"
		c.run(f"cp build/install/lib/{lib_file} cconfigspace/{lib_file}")

		if wheel:
			c.run("python3 setup.py bdist_wheel")

			if install:
				c.run("pip3 install dist/*.whl")

		else:

			if dev:
				c.run("pip3 install -e.")
			else:
				c.run("python3 setup.py install")
