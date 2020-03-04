AM_COLOR_TESTS = yes

AM_CFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS = ../src/libcconfigspace.la

@VALGRIND_CHECK_RULES@

RNG_TESTS = \
		test_rng

# unit tests
UNIT_TESTS = \
		 $(RNG_TESTS)

# all tests
TST_PROGS = $(UNIT_TESTS)
check_PROGRAMS = $(TST_PROGS)
TESTS = $(TST_PROGS)
