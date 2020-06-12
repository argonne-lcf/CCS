AM_CPPFLAGS = -I$(top_srcdir)/include

AM_CFLAGS = -Wall -Werror -Wpedantic

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
			hyperparameter.c \
			hyperparameter_internal.h \
			hyperparameter_numerical.c \
			uthash.h \
			datum_hash.h \
			hyperparameter_categorical.c \
			hyperparameter_ordinal.c \
			utarray.h \
			context.c \
			context_internal.h \
			configuration_space.c \
			configuration_space_internal.h \
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
			tuner_random.c

@VALGRIND_CHECK_RULES@
