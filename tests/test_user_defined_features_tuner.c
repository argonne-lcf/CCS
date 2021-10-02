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

struct tuner_last_s {
	ccs_features_evaluation_t last_eval;
};
typedef struct tuner_last_s tuner_last_t;

ccs_result_t
tuner_last_del(ccs_features_tuner_t tuner) {
	tuner_last_t *tuner_data;
	ccs_result_t err;
	err = ccs_user_defined_features_tuner_get_tuner_data(tuner, (void**)&tuner_data);
	if (err)
		return err;
	if (tuner_data && tuner_data->last_eval)
		ccs_release_object(tuner_data->last_eval);
	free(tuner_data);
	return CCS_SUCCESS;
}

ccs_result_t
tuner_last_ask(ccs_features_tuner_t  tuner,
               ccs_features_t        features,
               size_t                num_configurations,
               ccs_configuration_t  *configurations,
               size_t               *num_configurations_ret) {
	(void) features;
	if (!configurations) {
		*num_configurations_ret = 1;
		return CCS_SUCCESS;
	}
	ccs_result_t err;
        ccs_configuration_space_t configuration_space;
	err = ccs_features_tuner_get_configuration_space(tuner, &configuration_space);
	if (err)
		return err;
	err = ccs_configuration_space_samples(configuration_space,
	                                      num_configurations, configurations);
	if (err)
		return err;
	if (num_configurations_ret)
		*num_configurations_ret = num_configurations;
	return CCS_SUCCESS;
}

ccs_result_t
tuner_last_tell(ccs_features_tuner_t       tuner,
                size_t                     num_evaluations,
                ccs_features_evaluation_t *evaluations) {
	if (!num_evaluations)
		return CCS_SUCCESS;
	ccs_result_t err;
	tuner_last_t *tuner_data;
	err = ccs_user_defined_features_tuner_get_tuner_data(tuner, (void**)&tuner_data);
	if (err)
		return err;
	err = ccs_retain_object(evaluations[num_evaluations - 1]);
	if (err)
		return err;
	if (tuner_data->last_eval)
		ccs_release_object(tuner_data->last_eval);
	tuner_data->last_eval = evaluations[num_evaluations - 1];
	return CCS_SUCCESS;
}

ccs_result_t
tuner_last_get_optimums(ccs_features_tuner_t       tuner,
                        ccs_features_t             features,
                        size_t                     num_evaluations,
                        ccs_features_evaluation_t *evaluations,
                        size_t                    *num_evaluations_ret) {
	size_t count = 0;
	if (evaluations) {
		ccs_result_t err;
		tuner_last_t *tuner_data;
		err = ccs_user_defined_features_tuner_get_tuner_data(
			tuner, (void**)&tuner_data);
		if (err)
			return err;
		if (features) {
			ccs_features_t feat;
			int            cmp;
			err = ccs_features_evaluation_get_features(tuner_data->last_eval,
			                                           &feat);
			if (err)
				return err;
			err = ccs_features_cmp(features, feat, &cmp); 
			if (err)
				return err;
			if (cmp == 0) {
				if (num_evaluations < 1)
					return -CCS_INVALID_VALUE;
				count = 1;
				evaluations[0] = tuner_data->last_eval;
			}
		} else {
			if (num_evaluations < 1)
				return -CCS_INVALID_VALUE;
			count = 1;
			evaluations[0] = tuner_data->last_eval;
		}
		for(size_t i = count; i < num_evaluations; i++)
			evaluations[i] = NULL;
	}
	if (num_evaluations_ret)
		*num_evaluations_ret = count;
	return CCS_SUCCESS;
}

ccs_result_t
tuner_last_get_history(ccs_features_tuner_t       tuner,
                       ccs_features_t             features,
                       size_t                     num_evaluations,
                       ccs_features_evaluation_t *evaluations,
                       size_t                    *num_evaluations_ret) {
	return tuner_last_get_optimums(tuner, features, num_evaluations,
	                               evaluations, num_evaluations_ret);
}

ccs_user_defined_features_tuner_vector_t tuner_last_vector = {
	&tuner_last_del,
	&tuner_last_ask,
	&tuner_last_tell,
	&tuner_last_get_optimums,
	&tuner_last_get_history,
	NULL,
};

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
	tuner_last_t              *tuner_data;
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

	tuner_data = (tuner_last_t *)calloc(1, sizeof(tuner_last_t));
	assert( tuner_data );

	err = ccs_create_user_defined_features_tuner(
		"problem", cspace, fspace, ospace, NULL,
		&tuner_last_vector, tuner_data, &tuner);
	assert( err == CCS_SUCCESS );

	ccs_features_evaluation_t    last_evaluation;
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
		last_evaluation = evaluation;
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
		last_evaluation = evaluation;
		err = ccs_release_object(configuration);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(evaluation);
		assert( err == CCS_SUCCESS );
	}

	size_t           count;
	ccs_features_evaluation_t history[100];
	err = ccs_features_tuner_get_history(tuner, NULL, 100, history, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 1 );

	ccs_features_evaluation_t evaluation;
	err = ccs_features_tuner_get_optimums(tuner, NULL, 1, &evaluation, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 1 );
	assert( last_evaluation == evaluation );

	err = ccs_features_tuner_get_optimums(tuner, features_on, 1, &evaluation, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 0 );

	err = ccs_features_tuner_get_optimums(tuner, features_off, 1, &evaluation, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 1 );

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

int main() {
	ccs_init();
	test();
	ccs_fini();
	return 0;
}

