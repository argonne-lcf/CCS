# SYNOPSIS
#
#  AX_DYNAMIC_VERSION()
#
# DESCRIPTION
#
#  AX_DYNAMIC_VERSION will use `git-version-gen` to produce a dynamic version
#  number that can be used at compile time. It defines DYNAMIC_VERSION_RULES
#  which should be substituted in your makefile. It will populate a CURVER
#  variable that will contain the version number (backed by a .version file),
#  and a timestamp file to use as a dependency in rules, `.version_timestamp`.
#  It also defaults to using $(top_srcdir)/VERSION as a source for the version
#  number should `git-version-gen` find neither git or `.tarball-version`, or
#  fails altogether. The $(top_srcdir)/VERSION must contain a version number
#  in vX.Y.Z.W format.
#
#  Usage example:
#
#  configure.ac
#
#    AX_DYNAMIC_VERSION
#
#  in each Makefile.am requiring the version number:
#
#    @DYNAMIC_VERSION_RULES@
#
#  This results in a CURVER variable being defined that will contain the version
#  number when evaluated, and a `.version_timestamp` file that can be used as a
#  dependency for rules that depend on this version number.

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
