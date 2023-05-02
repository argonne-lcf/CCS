AM_CPPFLAGS = -I$(top_srcdir)/include

AM_CFLAGS = -Wall -Wextra -Wpedantic $(GSL_CFLAGS)
AM_LDFLAGS = $(GSL_LIBS)

if STRICT
AM_CFLAGS += -Werror
endif

lib_LTLIBRARIES = libcconfigspace.la

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
	  echo "#ifndef CCS_VERSION_H_" > version.h; \
	  echo "#define CCS_VERSION_H_" >> version.h; \
	  echo "#define CCS_VERSION_STRING  \"v$(OLDVER)\"" >> version.h; \
	  echo "#define CCS_VERSION_MAJOR $$(echo "$(OLDVER)" | cut -d . -f 1)" >> version.h; \
	  echo "#define CCS_VERSION_MINOR $$(echo "$(OLDVER)" | cut -d . -f 2)" >> version.h; \
	  echo "#define CCS_VERSION_PATCH $$(echo "$(OLDVER)" | cut -d . -f 3)" >> version.h; \
	  echo "#define CCS_VERSION_REVISION $$(echo "$(OLDVER)" | cut -d . -f 4 | cut -d - -f 1 )" >> version.h; \
	  echo "#endif" >> version.h; \
	fi

version.h: .version

BUILT_SOURCES = \
	version.h

nodist_libcconfigspace_la_SOURCES = \
	version.h

CLEANFILES = \
	.version \
	version.h

libcconfigspace_la_SOURCES = \
	uthash.h \
	utarray.h \
	utlist.h \
	cconfigspace.c \
	cconfigspace_internal.h \
	error_stack.c \
	error_stack_internal.h \
	interval.c \
	rng.c \
	rng_internal.h \
	rng_deserialize.h \
	distribution.c \
	distribution_internal.h \
	distribution_deserialize.h \
	distribution_uniform.c \
	distribution_normal.c \
	distribution_roulette.c \
	distribution_mixture.c \
	distribution_multivariate.c \
	datum_uthash.h \
	datum_hash.h \
	parameter.c \
	parameter_internal.h \
	parameter_deserialize.h \
	parameter_numerical.c \
	parameter_string.c \
	parameter_categorical.c \
	context.c \
	context_internal.h \
	context_deserialize.h \
	configuration_space.c \
	configuration_space_internal.h \
	configuration_space_deserialize.h \
	binding.c \
	binding_internal.h \
	configuration.c \
	configuration_internal.h \
	configuration_deserialize.h \
	expression.c \
	expression_internal.h \
	expression_deserialize.h \
	objective_space.c \
	objective_space_internal.h \
	objective_space_deserialize.h \
	evaluation.c \
	evaluation_internal.h \
	evaluation_deserialize.h \
	tuner.c \
	tuner_internal.h \
	tuner_deserialize.h \
	tuner_random.c \
	tuner_user_defined.c \
	features_space.c \
	features_space_internal.h \
	features_space_deserialize.h \
	features.c \
	features_internal.h \
	features_deserialize.h \
	features_evaluation.c \
	features_evaluation_internal.h \
	features_evaluation_deserialize.h \
	features_tuner.c \
	features_tuner_internal.h \
	features_tuner_deserialize.h \
	features_tuner_random.c \
	features_tuner_user_defined.c \
	map.c \
	map_internal.h \
	map_deserialize.h \
	tree.c \
	tree_internal.h \
	tree_deserialize.h \
	tree_space.c \
	tree_space_internal.h \
	tree_space_deserialize.h \
	tree_space_static.c \
	tree_space_dynamic.c \
	tree_configuration.c \
	tree_configuration_internal.h \
	tree_configuration_deserialize.h \
	tree_evaluation.c \
	tree_evaluation_internal.h \
	tree_evaluation_deserialize.h \
	tree_tuner.c \
	tree_tuner_internal.h \
	tree_tuner_deserialize.h \
	tree_tuner_random.c \
	tree_tuner_user_defined.c

@VALGRIND_CHECK_RULES@
