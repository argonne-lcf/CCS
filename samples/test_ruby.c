#include <stdlib.h>
#include <cconfigspace.h>
#include <string.h>
// Ruby 3+ bleeds NDEBUG
// see https://bugs.ruby-lang.org/issues/18777#change-97580
#ifdef NDEBUG
#include <ruby.h>
#else
#include <ruby.h>
#undef NDEBUG
#endif
// DO NOT MOVE. If assert.h is included before ruby.h asserts will turn to
// no-op.
#include <assert.h>

ccs_parameter_t
create_numerical(const char *name, double lower, double upper)
{
	ccs_parameter_t parameter;
	ccs_error_t     err;
	err = ccs_create_numerical_parameter(
		name,
		CCS_NUM_FLOAT,
		CCSF(lower),
		CCSF(upper),
		CCSF(0.0),
		CCSF(0),
		&parameter);
	assert(err == CCS_SUCCESS);
	return parameter;
}

void
create_problem(ccs_configuration_space_t *cs, ccs_objective_space_t *os)
{
	ccs_parameter_t           parameter1, parameter2;
	ccs_parameter_t           parameter3;
	ccs_configuration_space_t cspace;
	ccs_objective_space_t     ospace;
	ccs_expression_t          expression;
	ccs_error_t               err;

	parameter1 = create_numerical("x", -5.0, 5.0);
	parameter2 = create_numerical("y", -5.0, 5.0);

	err        = ccs_create_configuration_space("2dplane", &cspace);
	assert(err == CCS_SUCCESS);
	err = ccs_configuration_space_add_parameter(cspace, parameter1, NULL);
	assert(err == CCS_SUCCESS);
	err = ccs_configuration_space_add_parameter(cspace, parameter2, NULL);
	assert(err == CCS_SUCCESS);

	parameter3 = create_numerical("z", -CCS_INFINITY, CCS_INFINITY);
	err        = ccs_create_variable(parameter3, &expression);
	assert(err == CCS_SUCCESS);

	err = ccs_create_objective_space("height", &ospace);
	assert(err == CCS_SUCCESS);
	err = ccs_objective_space_add_parameter(ospace, parameter3);
	assert(err == CCS_SUCCESS);
	err = ccs_objective_space_add_objective(
		ospace, expression, CCS_MINIMIZE);
	assert(err == CCS_SUCCESS);

	err = ccs_release_object(parameter1);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(parameter2);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(parameter3);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_SUCCESS);

	*cs = cspace;
	*os = ospace;
}

void
test_tuner(ccs_tuner_t tuner, ccs_objective_space_t ospace)
{
	ccs_error_t err;

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t         values[2], res;
		ccs_configuration_t configuration;
		ccs_evaluation_t    evaluation;
		err = ccs_tuner_ask(tuner, 1, &configuration, NULL);
		assert(err == CCS_SUCCESS);
		err = ccs_configuration_get_values(
			configuration, 2, values, NULL);
		assert(err == CCS_SUCCESS);
		res = ccs_float(
			(values[0].value.f - 1) * (values[0].value.f - 1) +
			(values[1].value.f - 2) * (values[1].value.f - 2));
		ccs_create_evaluation(
			ospace,
			configuration,
			CCS_SUCCESS,
			1,
			&res,
			&evaluation);
		err = ccs_tuner_tell(tuner, 1, &evaluation);
		assert(err == CCS_SUCCESS);
		err = ccs_release_object(configuration);
		assert(err == CCS_SUCCESS);
		err = ccs_release_object(evaluation);
		assert(err == CCS_SUCCESS);
	}

	size_t           count;
	ccs_evaluation_t history[100];
	ccs_datum_t      min = ccs_float(INFINITY);
	err = ccs_tuner_get_history(tuner, 100, history, &count);
	assert(err == CCS_SUCCESS);
	assert(count == 100);

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t res;
		err = ccs_evaluation_get_objective_value(history[i], 0, &res);
		assert(err == CCS_SUCCESS);
		if (res.value.f < min.value.f)
			min.value.f = res.value.f;
	}

	ccs_evaluation_t evaluation;
	ccs_datum_t      res;
	err = ccs_tuner_get_optimums(tuner, 1, &evaluation, NULL);
	assert(err == CCS_SUCCESS);
	err = ccs_evaluation_get_objective_value(evaluation, 0, &res);
	assert(res.value.f == min.value.f);
}

void
test()
{
	ccs_tuner_t               t;
	ccs_configuration_space_t cs;
	ccs_objective_space_t     os;
	ccs_error_t               err;
	int                       state;
	VALUE                     ruby_stack_start;

	create_problem(&cs, &os);

	ruby_init_stack(&ruby_stack_start);
	ruby_init();
	{
		int         dummy_argc   = 2;
		const char *dummy_argv[] = {"ccs-ruby", "-e_=0"};
		ruby_options(dummy_argc, (char **)dummy_argv);
	}
	ruby_script("test_ruby");
	ruby_init_loadpath();
	rb_eval_string_protect("require 'rubygems'", &state);
	if (state) {
		ruby_cleanup(state);
		exit(1);
	}
	rb_eval_string_protect("require_relative './test_ruby.rb'", &state);
	if (state) {
		ruby_cleanup(state);
		exit(1);
	}
	VALUE tuner;
	tuner = rb_funcall(
		rb_current_receiver(),
		rb_intern("create_test_tuner"),
		2,
		ULL2NUM((uintptr_t)cs),
		ULL2NUM((uintptr_t)os));
	t = (ccs_tuner_t)NUM2ULL(rb_funcall(
		rb_funcall(tuner, rb_intern("handle"), 0),
		rb_intern("to_i"),
		0));
	ccs_retain_object(t);
	test_tuner(t, os);
	err = ccs_release_object(t);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(os);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(cs);
	assert(err == CCS_SUCCESS);
	ruby_cleanup(0);
}

int
main()
{
	ccs_init();
	test();
	return 0;
}
