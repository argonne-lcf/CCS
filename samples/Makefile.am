AM_COLOR_TESTS = yes

test_ruby_CFLAGS = -I$(top_srcdir)/include -Wall -Werror -Wpedantic $(RUBY_CFLAGS) -Wno-deprecated-declarations
test_ruby_LDFLAGS = ../src/libcconfigspace.la $(RUBY_LIBS)

test_python_CFLAGS = -I$(top_srcdir)/include -Wall -Werror -Wpedantic $(PYTHON_CFLAGS)
test_python_LDFLAGS = ../src/libcconfigspace.la $(PYTHON_LIBS)

CCONFIGSPACE_TESTS = \
		test_ruby \
		test_python

# unit tests
UNIT_TESTS = \
		 $(CCONFIGSPACE_TESTS)

# all tests
TEST_PROGS = $(UNIT_TESTS)
check_PROGRAMS = $(TEST_PROGS)
TESTS = $(TEST_PROGS)
