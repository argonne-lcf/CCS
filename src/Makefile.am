AM_CPPFLAGS = -I$(top_srcdir)/include

AM_CFLAGS = -Wall -Wextra -Wpedantic $(GSL_CFLAGS)
AM_LDFLAGS = $(GSL_LIBS)

if STRICT
AM_CFLAGS += -Werror
endif

if THREAD_SAFE
AM_CFLAGS += -DTHREAD_SAFE
endif

lib_LTLIBRARIES = libcconfigspace.la

version.h: $(srcdir)/version.h.subst .version_timestamp
	@echo Processing $@; sh $(top_srcdir)/build-aux/version-subst $(CURVER) $< $@

EXTRA_DIST = \
	version.h.subst

BUILT_SOURCES = \
	version.h

nodist_libcconfigspace_la_SOURCES = \
	version.h

CLEANFILES = \
	version.h

libcconfigspace_la_SOURCES = \
	uthash.h \
	utarray.h \
	utlist.h \
	cconfigspace.c \
	cconfigspace_internal.h \
	cconfigspace_deserialize.h \
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
	search_space_internal.h \
	configuration_space.c \
	configuration_space_internal.h \
	configuration_space_deserialize.h \
	distribution_space.c \
	distribution_space_internal.h \
        distribution_space_deserialize.h \
	binding.c \
	binding_internal.h \
	search_configuration_internal.h \
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
	feature_space.c \
	feature_space_internal.h \
	feature_space_deserialize.h \
	features.c \
	features_internal.h \
	features_deserialize.h \
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
	tree_configuration_deserialize.h

@DYNAMIC_VERSION_RULES@

@VALGRIND_CHECK_RULES@
