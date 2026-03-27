ACLOCAL_AMFLAGS = -I m4
include $(top_srcdir)/aminclude_static.am
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

DISTCLEANFILES = aminclude_static.am

CODE_COVERAGE_LCOV_SHOPTS = --ignore-errors inconsistent $(CODE_COVERAGE_LCOV_SHOPTS_DEFAULT)
CODE_COVERAGE_LCOV_RMOPTS = --ignore-errors inconsistent $(CODE_COVERAGE_LCOV_RMOPTS_DEFAULT)

clean-local: code-coverage-clean
distclean-local: code-coverage-dist-clean

EXTRA_DIST = \
	autogen.sh \
	cconfigspace.pc.subst \
	README.md \
	build-aux/git-version-gen \
	build-aux/version-subst

dist-hook: .version
	@echo Creating ditribution .tarball-version; echo v$(CURVER) > $(distdir)/.tarball-version

@DYNAMIC_VERSION_RULES@
