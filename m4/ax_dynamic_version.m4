AC_DEFUN([AX_DYNAMIC_VERSION],[

[DYNAMIC_VERSION_RULES='
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
	  echo "$(CURVER)" > .version; \
	  touch .version_timestamp; \
	fi

.version_timestamp: .version

CLEANFILES ?=
CLEANFILES += .version .version_timestamp
']
AC_SUBST([DYNAMIC_VERSION_RULES])
m4_ifdef([_AM_SUBST_NOTMAKE], [_AM_SUBST_NOTMAKE([DYNAMIC_VERSION_RULES])])
])
