from setuptools import setup, find_packages
from pkg_resources import require
require("parglare>=0.12.0")
setup(
    name = "cconfigspace",
    version = "0.0.1",
    author = "Brice Videau",
    author_email = "bvideau@anl.gov",
    url = "https://xgitlab.cels.anl.gov/videau/cconfigspace",
    packages = ["cconfigspace"],
    package_dir={'cconfigspace': 'cconfigspace'},
    license='BSD-3-Clause'
    )
