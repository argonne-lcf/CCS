ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src include tests connectors
if SAMPLES
SUBDIRS += samples
endif
SUBDIRS += bindings

GITVER = $$(cd $(top_srcdir) 2>/dev/null && sh build-aux/git-version-gen --fallback `cat VERSION` .tarball-version 2>/dev/null)
OLDVER = $$(cat .version 2>/dev/null)
CURVER = $$(if [ -z "$(GITVER)" ]; then cat $(top_srcdir)/VERSION | cut -c2- 2>/dev/null; else echo "$(GITVER)"; fi)

.PHONY: .version
.version: $(top_srcdir)/VERSION
	@if [ "$(CURVER)" = "$(OLDVER)" ]; \
	then \
	  echo "Version up to date: $(CURVER)"; \
	else \
	  echo "Updating version to: $(CURVER)"; \
	  echo "$(CURVER)"> $@; \
	fi

cconfigspace.pc: cconfigspace.pc.subst .version
	@echo Processing $@; sed -e "s/==VERSION==/$(CURVER)/" $< > $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = cconfigspace.pc

CLEANFILES = \
	.version \
	cconfigspace.pc

EXTRA_DIST = autogen.sh cconfigspace.pc.subst README.md
