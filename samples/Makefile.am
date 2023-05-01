AM_COLOR_TESTS = yes

test_ruby_CFLAGS = -I$(top_srcdir)/include -Wall -Wextra $(RUBY_CFLAGS) -Wno-deprecated-declarations

if !ISMACOS
if STRICT
test_ruby_CFLAGS += -Werror
endif
endif

test_ruby_LDFLAGS = ../src/libcconfigspace.la $(RUBY_LIBS)

test_python_CFLAGS = -I$(top_srcdir)/include -Wall -Wextra -Wpedantic $(PYTHON_CFLAGS) -DBINDINGS_PATH="\"$(top_srcdir)/bindings/python\""

if STRICT
test_python_CFLAGS += -Werror
endif

test_python_LDFLAGS = ../src/libcconfigspace.la $(PYTHON_LIBS)

CCONFIGSPACE_TESTS = \
		test_ruby \
		test_python

clean-local:
	$(RM) -fr __pycache__

# unit tests
UNIT_TESTS = \
		 $(CCONFIGSPACE_TESTS)

# all tests
TEST_PROGS = $(UNIT_TESTS)
check_PROGRAMS = $(TEST_PROGS)
TESTS = $(TEST_PROGS)
