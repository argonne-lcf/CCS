EXTRA_DIST = \
	lib/cconfigspace.rb \
	lib/cconfigspace \
	lib/cconfigspace/features_tuner.rb \
	lib/cconfigspace/tree_evaluation.rb \
	lib/cconfigspace/tree.rb \
	lib/cconfigspace/evaluation.rb \
	lib/cconfigspace/tuner.rb \
	lib/cconfigspace/tree_tuner.rb \
	lib/cconfigspace/binding.rb \
	lib/cconfigspace/tree_space.rb \
	lib/cconfigspace/features_space.rb \
	lib/cconfigspace/base.rb \
	lib/cconfigspace/configuration_space.rb \
	lib/cconfigspace/configuration.rb \
	lib/cconfigspace/features.rb \
	lib/cconfigspace/distribution.rb \
	lib/cconfigspace/rng.rb \
	lib/cconfigspace/map.rb \
	lib/cconfigspace/expression.rb \
	lib/cconfigspace/expression_parser.rb \
	lib/cconfigspace/objective_space.rb \
	lib/cconfigspace/features_evaluation.rb \
	lib/cconfigspace/interval.rb \
	lib/cconfigspace/parameter.rb \
	lib/cconfigspace/error_stack.rb \
	lib/cconfigspace/tree_configuration.rb \
	lib/cconfigspace/context.rb \
	test/test_features_space.rb \
	test/test_expression.rb \
	test/test_tree_evaluation.rb \
	test/test_interval.rb \
	test/test_expression_parser.rb \
	test/test_tree_space.rb \
	test/test_parameter.rb \
	test/test_objective_space.rb \
	test/test_tree.rb \
	test/test_rng.rb \
	test/test_base.rb \
	test/test_tuner.rb \
	test/test_tree_tuner.rb \
	test/test_features_tuner.rb \
	test/test_configuration_space.rb \
	test/test_distribution.rb \
	test/test_evaluation.rb \
	rakefile \
	cconfigspace.gemspec \
	LICENSE

if ISMACOS
LIBEXT = .dylib
else
LIBEXT = .so
endif

check: FORCE
	LIBCCONFIGSPACE_SO=$(top_builddir)/src/.libs/libcconfigspace$(LIBEXT) SRC_DIR=$(srcdir) rake -f $(srcdir)/rakefile test

FORCE:
