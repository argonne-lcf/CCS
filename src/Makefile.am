AM_CPPFLAGS = -I$(top_srcdir)/include

AM_CFLAGS = -Wall -Werror -Wpedantic

lib_LTLIBRARIES = libcconfigspace.la

libcconfigspace_la_SOURCES = \
			cconfigspace.c \
			cconfigspace_internal.h \
			rng.c \
			rng_internal.h \
			distribution.c \
			distribution_internal.h \
			distribution_uniform.c \
			distribution_normal.c \
			hyperparameter.c \
			hyperparameter_internal.h \
			hyperparameter_numerical.c

@VALGRIND_CHECK_RULES@
