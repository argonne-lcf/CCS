AM_COLOR_TESTS = yes

AM_CFLAGS = -I$(top_srcdir)/include -Wall -Werror -Wpedantic
AM_LDFLAGS = ../src/libcconfigspace.la

RNG_TESTS = \
		test_rng \
		test_interval \
		test_uniform_distribution \
		test_normal_distribution \
		test_roulette_distribution \
		test_numerical_hyperparameter \
		test_categorical_hyperparameter \
		test_ordinal_hyperparameter

# unit tests
UNIT_TESTS = \
		 $(RNG_TESTS)

# all tests
TST_PROGS = $(UNIT_TESTS)
check_PROGRAMS = $(TST_PROGS)
TESTS = $(TST_PROGS)

@VALGRIND_CHECK_RULES@
