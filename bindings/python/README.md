# CConfigSpace (Python)

## Installation

### MacOS (x86/arm)

Install the GSL and Automake libraries with Homebrew:

```console
brew install gsl automake
```

<!-- gem install --user-install rake ffi whittle -->

Install `invoke` (Python version of `make`):

```console
pip install invoke
```

Make sure to set up your brew environment correctly to have access to the GSL library:

```console
export LD_LIBRARY_PATH=$(brew --prefix)/lib:$LD_LIBRARY_PATH
export LIBRARY_PATH=$(brew --prefix)/lib:$LIBRARY_PATH
export CPATH=$(brew --prefix)/include:$CPATH
```

Build the CConfigSpace C library:

```console
invoke ccs-build
```

Install the Python binding:

```
pip install -e.
```