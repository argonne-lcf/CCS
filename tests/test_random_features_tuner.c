#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

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


void test() {
	ccs_hyperparameter_t      hyperparameter1, hyperparameter2;
	ccs_hyperparameter_t      hyperparameter3;
	ccs_hyperparameter_t      feature;
	ccs_configuration_space_t cspace;
	ccs_features_space_t      fspace;
	ccs_objective_space_t     ospace;
	ccs_expression_t          expression;
	ccs_features_tuner_t      tuner;
	ccs_result_t              err;
	ccs_features_t            features_on, features_off;
	ccs_datum_t               knobs_values[2] =
		{ ccs_string("on"), ccs_string("off") };

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

	err = ccs_create_categorical_hyperparameter("red knob", 2, knobs_values, 0,
	                                            NULL, &feature);
	assert( err == CCS_SUCCESS );

	err = ccs_create_features_space("knobs", NULL, &fspace);
	assert( err == CCS_SUCCESS );
	err = ccs_features_space_add_hyperparameter(fspace, feature);
	assert( err == CCS_SUCCESS );
	err = ccs_create_features(fspace, 1, knobs_values, NULL, &features_on);
	assert( err == CCS_SUCCESS );
	err = ccs_create_features(fspace, 1, knobs_values + 1, NULL, &features_off);
	assert( err == CCS_SUCCESS );

	err = ccs_create_random_features_tuner("problem", cspace, fspace, ospace,
	                                       NULL, &tuner);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < 50; i++) {
		ccs_datum_t         values[2], res;
		ccs_configuration_t       configuration;
		ccs_features_evaluation_t evaluation;
		err = ccs_features_tuner_ask(tuner, features_on, 1, &configuration, NULL);
		assert( err == CCS_SUCCESS );
		err = ccs_configuration_get_values(configuration, 2, values, NULL);
		assert( err == CCS_SUCCESS );
		res = ccs_float((values[0].value.f - 1)*(values[0].value.f - 1) +
		                (values[1].value.f - 2)*(values[1].value.f - 2));
		err = ccs_create_features_evaluation(ospace, configuration, features_on,
		                               CCS_SUCCESS, 1, &res, NULL, &evaluation);
		assert( err == CCS_SUCCESS );
		err = ccs_features_tuner_tell(tuner, 1, &evaluation);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(configuration);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(evaluation);
		assert( err == CCS_SUCCESS );
	}

	for (size_t i = 0; i < 50; i++) {
		ccs_datum_t         values[2], res;
		ccs_configuration_t       configuration;
		ccs_features_evaluation_t evaluation;
		err = ccs_features_tuner_ask(tuner, features_off, 1, &configuration, NULL);
		assert( err == CCS_SUCCESS );
		err = ccs_configuration_get_values(configuration, 2, values, NULL);
		assert( err == CCS_SUCCESS );
		res = ccs_float((values[0].value.f - 1)*(values[0].value.f - 1) +
		                (values[1].value.f - 2)*(values[1].value.f - 2));
		err = ccs_create_features_evaluation(ospace, configuration, features_off,
		                               CCS_SUCCESS, 1, &res, NULL, &evaluation);
		assert( err == CCS_SUCCESS );
		err = ccs_features_tuner_tell(tuner, 1, &evaluation);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(configuration);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(evaluation);
		assert( err == CCS_SUCCESS );
	}


	size_t                    count;
	ccs_features_evaluation_t history[100];
	ccs_features_evaluation_t evaluation;
	ccs_datum_t               min_on = ccs_float(INFINITY);
	ccs_datum_t               min_off = ccs_float(INFINITY);
	err = ccs_features_tuner_get_history(tuner, NULL, 0, NULL, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 100 );

	err = ccs_features_tuner_get_optimums(tuner, NULL, 0, NULL, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 2 );

	err = ccs_features_tuner_get_history(tuner, features_on, 50, history, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 50 );

	for (size_t i = 0; i < 50; i++) {
		ccs_datum_t      res;
		err = ccs_features_evaluation_get_objective_value(history[i], 0, &res);
		assert( err == CCS_SUCCESS );
		if (res.value.f < min_on.value.f)
			min_on.value.f = res.value.f;
	}

	err = ccs_features_tuner_get_history(tuner, features_off, 50, history, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 50 );

	for (size_t i = 0; i < 50; i++) {
		ccs_datum_t      res;
		err = ccs_features_evaluation_get_objective_value(history[i], 0, &res);
		assert( err == CCS_SUCCESS );
		if (res.value.f < min_off.value.f)
			min_off.value.f = res.value.f;
	}

	ccs_datum_t      res;
	err = ccs_features_tuner_get_optimums(tuner, features_on, 1, &evaluation, NULL);
	assert( err == CCS_SUCCESS );
	err = ccs_features_evaluation_get_objective_value(evaluation, 0, &res);
	assert( res.value.f == min_on.value.f );

	err = ccs_features_tuner_get_optimums(tuner, features_off, 1, &evaluation, NULL);
	assert( err == CCS_SUCCESS );
	err = ccs_features_evaluation_get_objective_value(evaluation, 0, &res);
	assert( res.value.f == min_off.value.f );

	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter3);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(feature);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(features_on);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(features_off);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(cspace);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(ospace);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(fspace);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(tuner);
	assert( err == CCS_SUCCESS );
}

int main(int argc, char *argv[]) {
	ccs_init();
	test();
	return 0;
}

