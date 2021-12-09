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
	ccs_evaluation_t last_eval;
};
typedef struct tuner_last_s tuner_last_t;

ccs_result_t
tuner_last_del(ccs_tuner_t tuner) {
	tuner_last_t *tuner_data;
	ccs_result_t err;
	err = ccs_user_defined_tuner_get_tuner_data(tuner, (void**)&tuner_data);
	if (err)
		return err;
	if (tuner_data && tuner_data->last_eval)
		ccs_release_object(tuner_data->last_eval);
	free(tuner_data);
	return CCS_SUCCESS;
}

ccs_result_t
tuner_last_ask(ccs_tuner_t          tuner,
               size_t               num_configurations,
               ccs_configuration_t *configurations,
               size_t              *num_configurations_ret) {
	if (!configurations) {
		*num_configurations_ret = 1;
		return CCS_SUCCESS;
	}
	ccs_result_t err;
        ccs_configuration_space_t configuration_space;
	err = ccs_tuner_get_configuration_space(tuner, &configuration_space);
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
tuner_last_tell(ccs_tuner_t       tuner,
                size_t            num_evaluations,
                ccs_evaluation_t *evaluations) {
	if (!num_evaluations)
		return CCS_SUCCESS;
	ccs_result_t err;
	tuner_last_t *tuner_data;
	err = ccs_user_defined_tuner_get_tuner_data(tuner, (void**)&tuner_data);
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
tuner_last_get_optimums(ccs_tuner_t       tuner,
                        size_t            num_evaluations,
                        ccs_evaluation_t *evaluations,
                        size_t           *num_evaluations_ret) {
	if (evaluations) {
		if (num_evaluations < 1)
			return -CCS_INVALID_VALUE;
		ccs_result_t err;
		tuner_last_t *tuner_data;
		err = ccs_user_defined_tuner_get_tuner_data(tuner, (void**)&tuner_data);
		if (err)
			return err;
		evaluations[0] = tuner_data->last_eval;
		for(size_t i = 1; i < num_evaluations; i++)
			evaluations[i] = NULL;
	}
	if (num_evaluations_ret)
		*num_evaluations_ret = 1;
	return CCS_SUCCESS;
}

ccs_result_t
tuner_last_get_history(ccs_tuner_t       tuner,
                       size_t            num_evaluations,
                       ccs_evaluation_t *evaluations,
                       size_t           *num_evaluations_ret) {
	if (evaluations) {
		if (num_evaluations < 1)
			return -CCS_INVALID_VALUE;
		ccs_result_t err;
		tuner_last_t *tuner_data;
		err = ccs_user_defined_tuner_get_tuner_data(tuner, (void**)&tuner_data);
		if (err)
			return err;
		evaluations[0] = tuner_data->last_eval;
		for(size_t i = 1; i < num_evaluations; i++)
			evaluations[i] = NULL;
	}
	if (num_evaluations_ret)
		*num_evaluations_ret = 1;
	return CCS_SUCCESS;
}

ccs_user_defined_tuner_vector_t tuner_last_vector = {
	&tuner_last_del,
	&tuner_last_ask,
	&tuner_last_tell,
	&tuner_last_get_optimums,
	&tuner_last_get_history,
	NULL
};

void test() {
	ccs_hyperparameter_t      hyperparameter1, hyperparameter2;
	ccs_hyperparameter_t      hyperparameter3;
	ccs_configuration_space_t cspace;
	ccs_objective_space_t     ospace;
	ccs_expression_t          expression;
	ccs_tuner_t               tuner;
	ccs_result_t              err;
	tuner_last_t              *tuner_data;

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

	tuner_data = (tuner_last_t *)calloc(1, sizeof(tuner_last_t));
	assert( tuner_data );

	err = ccs_create_user_defined_tuner("problem", cspace, ospace, NULL, &tuner_last_vector, tuner_data, &tuner);
	assert( err == CCS_SUCCESS );

	ccs_evaluation_t    last_evaluation;
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
		last_evaluation = evaluation;
		err = ccs_release_object(configuration);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(evaluation);
		assert( err == CCS_SUCCESS );
	}

	size_t           count;
	ccs_evaluation_t history[100];
	err = ccs_tuner_get_history(tuner, 100, history, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 1 );

	ccs_evaluation_t evaluation;
	err = ccs_tuner_get_optimums(tuner, 1, &evaluation, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 1 );
	assert( last_evaluation == evaluation );

	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter3);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(cspace);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(ospace);
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

