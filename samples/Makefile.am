AM_COLOR_TESTS = yes

AM_CFLAGS = -I$(top_srcdir)/include -Wall -Werror -Wpedantic $(RUBY_CFLAGS)
AM_LDFLAGS = ../src/libcconfigspace.la $(RUBY_LIBS)

CCONFIGSPACE_TESTS = \
		test_ruby

# unit tests
UNIT_TESTS = \
		 $(CCONFIGSPACE_TESTS)

# all tests
TEST_PROGS = $(UNIT_TESTS)
check_PROGRAMS = $(TEST_PROGS)
TESTS = $(TEST_PROGS)
