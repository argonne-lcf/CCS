AM_CPPFLAGS = -I$(top_srcdir)/include

AM_CFLAGS = -Wall -Wextra -Wpedantic

if STRICT
AM_CFLAGS += -Werror
endif

lib_LTLIBRARIES = libcconfigspace.la

libcconfigspace_la_SOURCES = \
	cconfigspace.c \
	cconfigspace_internal.h \
	interval.c \
	rng.c \
	rng_internal.h \
	distribution.c \
	distribution_internal.h \
	distribution_uniform.c \
	distribution_normal.c \
	distribution_roulette.c \
	distribution_mixture.c \
	distribution_multivariate.c \
	hyperparameter.c \
	hyperparameter_internal.h \
	hyperparameter_numerical.c \
	hyperparameter_string.c \
	uthash.h \
	datum_uthash.h \
	datum_hash.h \
	hyperparameter_categorical.c \
	utarray.h \
	context.c \
	context_internal.h \
	configuration_space.c \
	configuration_space_internal.h \
	binding.c \
	binding_internal.h \
	configuration.c \
	configuration_internal.h \
	expression.c \
	expression_internal.h \
	objective_space.c \
	objective_space_internal.h \
	evaluation.c \
	evaluation_internal.h \
	tuner.c \
	tuner_internal.h \
	tuner_random.c \
	tuner_user_defined.c \
	features_space.c \
	features_space_internal.h \
	features.c \
	features_internal.h \
	features_evaluation.c \
	features_evaluation_internal.h \
	features_tuner.c \
	features_tuner_internal.h \
	features_tuner_random.c \
	features_tuner_user_defined.c

@VALGRIND_CHECK_RULES@
