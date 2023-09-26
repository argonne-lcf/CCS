EXTRA_DIST = \
	cconfigspace/rng.py \
	cconfigspace/context.py \
	cconfigspace/parameter.py \
	cconfigspace/features_space.py \
	cconfigspace/distribution.py \
	cconfigspace/map.py \
	cconfigspace/expression_parser.py \
	cconfigspace/tree.py \
	cconfigspace/configuration.py \
	cconfigspace/error_stack.py \
	cconfigspace/tree_space.py \
	cconfigspace/__init__.py \
	cconfigspace/expression.py \
	cconfigspace/configuration_space.py \
	cconfigspace/distribution_space.py \
	cconfigspace/tree_configuration.py \
	cconfigspace/tree_tuner.py \
	cconfigspace/features.py \
	cconfigspace/interval.py \
	cconfigspace/objective_space.py \
	cconfigspace/features_evaluation.py \
	cconfigspace/tree_evaluation.py \
	cconfigspace/binding.py \
	cconfigspace/features_tuner.py \
	cconfigspace/tuner.py \
	cconfigspace/base.py \
	cconfigspace/evaluation.py \
	test/test_distribution.py \
	test/test_tree_tuner.py \
	test/test_tree.py \
	test/test_evaluation.py \
	test/test_tree_evaluation.py \
	test/test_parameter.py \
	test/test_tree_space.py \
	test/test_base.py \
	test/test_interval.py \
	test/test_expression.py \
	test/test_tuner.py \
	test/test_expression_parser.py \
	test/test_rng.py \
	test/test_features_tuner.py \
	test/test_objective_space.py \
	test/test_configuration_space.py \
	test/test_features_space.py \
	setup.py

if ISMACOS
LIBEXT = .dylib
else
LIBEXT = .so
endif

check: FORCE
	PYTHONPATH=$(srcdir)/ LIBCCONFIGSPACE_SO_=$(top_builddir)/src/.libs/libcconfigspace$(LIBEXT) python3 -m unittest discover -s $(srcdir)/test/

FORCE:
