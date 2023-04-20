ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src include tests connectors
if SAMPLES
SUBDIRS += samples
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = cconfigspace.pc

EXTRA_DIST = autogen.sh cconfigspace.pc README.md
