AM_CPPFLAGS = -I$(top_srcdir)/include

AM_CFLAGS = -Wall -Werror -pedantic

lib_LTLIBRARIES = libcconfigspace.la

libcconfigspace_la_SOURCES = \
			cconfigspace.c \
			cconfigspace_internal.h \
			rng.c \
			rng_internal.h \
			distribution.c \
			distribution_internal.h \
			distribution_uniform.c

@VALGRIND_CHECK_RULES@
