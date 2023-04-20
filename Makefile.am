ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src include tests connectors
if SAMPLES
SUBDIRS += samples
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = cconfigspace.pc

EXTRA_DIST = autogen.sh cconfigspace.pc README.md

EXTRA_DIST += $(top_srcdir)/.version
BUILT_SOURCES = $(top_srcdir)/.version
$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@
dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version
