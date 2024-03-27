ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src include tests connectors bindings
if SAMPLES
SUBDIRS += samples
endif

cconfigspace.pc: cconfigspace.pc.subst .version_timestamp
	@echo Processing $@; sh $(top_srcdir)/build-aux/version-subst $(CURVER) $< $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = cconfigspace.pc

CLEANFILES = \
	cconfigspace.pc

EXTRA_DIST = \
	autogen.sh \
	cconfigspace.pc.subst \
	README.md \
	build-aux/git-version-gen \
	build-aux/version-subst

dist-hook: .version
	@echo Creating ditribution .tarball-version; echo v$(CURVER) > $(distdir)/.tarball-version

@DYNAMIC_VERSION_RULES@
