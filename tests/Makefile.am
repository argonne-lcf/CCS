AM_COLOR_TESTS = yes

AM_CFLAGS = -I$(top_srcdir)/include -Wall -Wextra -Wpedantic $(GSL_CFLAGS)

if STRICT
AM_CFLAGS += -Werror
endif

AM_LDFLAGS = ../src/libcconfigspace.la $(GSL_LIBS)

CCONFIGSPACE_TESTS = \
	test_rng \
	test_interval \
	test_uniform_distribution \
	test_normal_distribution \
	test_roulette_distribution \
	test_multivariate_distribution \
	test_mixture_distribution \
	test_numerical_hyperparameter \
	test_categorical_hyperparameter \
	test_ordinal_hyperparameter \
	test_discrete_hyperparameter \
	test_string_hyperparameter \
	test_configuration_space \
	test_expression \
	test_condition \
	test_forbidden \
	test_random_tuner \
	test_user_defined_tuner \
	test_features_space \
	test_random_features_tuner \
	test_user_defined_features_tuner \
	test_map \
	test_error_stack \
	test_tree \
	test_static_tree_space \
	test_dynamic_tree_space

# unit tests
UNIT_TESTS = \
		 $(CCONFIGSPACE_TESTS)

# all tests
TEST_PROGS = $(UNIT_TESTS)
check_PROGRAMS = $(TEST_PROGS)
TESTS = $(TEST_PROGS)

VALGRIND_memcheck_FLAGS = --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all
@VALGRIND_CHECK_RULES@
