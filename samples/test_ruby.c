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
// DO NOT MOVE. If assert.h is included before ruby.h asserts will turn to no-op.
#include <assert.h>

ccs_hyperparameter_t create_numerical(const char * name, double lower, double upper) {
	ccs_hyperparameter_t hyperparameter;
	ccs_result_t         err;
	err = ccs_create_numerical_hyperparameter(name, CCS_NUM_FLOAT,
	                                          CCSF(lower), CCSF(upper),
	                                          CCSF(0.0), CCSF(0),
	                                          NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );
	return hyperparameter;
}

void create_problem(ccs_configuration_space_t *cs, ccs_objective_space_t *os) {
	ccs_hyperparameter_t      hyperparameter1, hyperparameter2;
	ccs_hyperparameter_t      hyperparameter3;
	ccs_configuration_space_t cspace;
	ccs_objective_space_t     ospace;
	ccs_expression_t          expression;
	ccs_result_t              err;

	hyperparameter1 = create_numerical("x", -5.0, 5.0);
	hyperparameter2 = create_numerical("y", -5.0, 5.0);

	err = ccs_create_configuration_space("2dplane", NULL, &cspace);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_add_hyperparameter(cspace, hyperparameter1, NULL);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_add_hyperparameter(cspace, hyperparameter2, NULL);
	assert( err == CCS_SUCCESS );

	hyperparameter3 = create_numerical("z", -CCS_INFINITY, CCS_INFINITY);
	err = ccs_create_variable(hyperparameter3, &expression);
	assert( err == CCS_SUCCESS );

	err = ccs_create_objective_space("height", NULL, &ospace);
	assert( err == CCS_SUCCESS );
	err = ccs_objective_space_add_hyperparameter(ospace, hyperparameter3);
	assert( err == CCS_SUCCESS );
	err = ccs_objective_space_add_objective(ospace, expression, CCS_MINIMIZE);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(hyperparameter1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter3);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );

	*cs = cspace;
	*os = ospace;
}

void test_tuner(ccs_tuner_t tuner, ccs_objective_space_t ospace) {
	ccs_result_t         err;

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t         values[2], res;
		ccs_configuration_t configuration;
		ccs_evaluation_t    evaluation;
		err = ccs_tuner_ask(tuner, 1, &configuration, NULL);
		assert( err == CCS_SUCCESS );
		err = ccs_configuration_get_values(configuration, 2, values, NULL);
		assert( err == CCS_SUCCESS );
		res = ccs_float((values[0].value.f - 1)*(values[0].value.f - 1) +
		                (values[1].value.f - 2)*(values[1].value.f - 2));
		ccs_create_evaluation(ospace, configuration, CCS_SUCCESS, 1, &res, NULL, &evaluation);
		err = ccs_tuner_tell(tuner, 1, &evaluation);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(configuration);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(evaluation);
		assert( err == CCS_SUCCESS );
	}

	size_t           count;
	ccs_evaluation_t history[100];
	ccs_datum_t      min = ccs_float(INFINITY);
	err = ccs_tuner_get_history(tuner, 100, history, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 100 );

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t      res;
		err = ccs_evaluation_get_objective_value(history[i], 0, &res);
		assert( err == CCS_SUCCESS );
		if (res.value.f < min.value.f)
			min.value.f = res.value.f;
	}

	ccs_evaluation_t evaluation;
	ccs_datum_t      res;
	err = ccs_tuner_get_optimums(tuner, 1, &evaluation, NULL);
	assert( err == CCS_SUCCESS );
	err = ccs_evaluation_get_objective_value(evaluation, 0, &res);
	assert( res.value.f == min.value.f );
}

void test() {
	ccs_tuner_t               t;
	ccs_configuration_space_t cs;
	ccs_objective_space_t     os;
	ccs_result_t              err;

	create_problem(&cs, &os);

	ruby_init();
	ruby_init_loadpath();
	ruby_script("test_ruby");
	int state;
	rb_eval_string_protect("require_relative './test_ruby.rb'", &state);
	if (state) {
		ruby_cleanup(state);
		return;
	}
	VALUE tuner;
	tuner = rb_funcall(rb_current_receiver(), rb_intern("create_test_tuner"),
	                   2, ULL2NUM((uintptr_t)cs), ULL2NUM((uintptr_t)os));
	t = (ccs_tuner_t)NUM2ULL(rb_funcall(rb_funcall(tuner, rb_intern("handle"), 0), rb_intern("to_i"), 0));
	ccs_retain_object(t);
	test_tuner(t, os);
	err = ccs_release_object(t);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(os);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(cs);
	assert( err == CCS_SUCCESS );
	ruby_cleanup(0);
}

int main() {
	ccs_init();
	test();
	return 0;
}
