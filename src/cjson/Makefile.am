noinst_LTLIBRARIES = libcjson.la

libcjson_la_SOURCES = \
	cJSON.c \
	cJSON.h

# Suppress warnings for vendored code — cJSON is C89 and may trigger
# warnings under CCS's -Wpedantic / -Werror flags
AM_CFLAGS = -w

EXTRA_DIST = LICENSE
