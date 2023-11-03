#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

ccs_parameter_t
create_numerical(const char *name, double lower, double upper)
{
	ccs_parameter_t parameter;
	ccs_result_t    err;
	err = ccs_create_numerical_parameter(
		name, CCS_NUMERIC_TYPE_FLOAT, CCSF(lower), CCSF(upper),
		CCSF(0.0), CCSF(0), &parameter);
	assert(err == CCS_RESULT_SUCCESS);
	return parameter;
}

struct tuner_last_s {
	ccs_features_evaluation_t last_eval;
};
typedef struct tuner_last_s tuner_last_t;

ccs_result_t
tuner_last_del(ccs_features_tuner_t tuner)
{
	tuner_last_t *tuner_data;
	ccs_result_t  err;
	err = ccs_user_defined_features_tuner_get_tuner_data(
		tuner, (void **)&tuner_data);
	if (err)
		return err;
	if (tuner_data && tuner_data->last_eval)
		ccs_release_object(tuner_data->last_eval);
	free(tuner_data);
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
tuner_last_ask(
	ccs_features_tuner_t tuner,
	ccs_features_t       features,
	size_t               num_configurations,
	ccs_configuration_t *configurations,
	size_t              *num_configurations_ret)
{
	(void)features;
	if (!configurations) {
		*num_configurations_ret = 1;
		return CCS_RESULT_SUCCESS;
	}
	ccs_result_t              err;
	ccs_configuration_space_t configuration_space;
	err = ccs_features_tuner_get_configuration_space(
		tuner, &configuration_space);
	if (err)
		return err;
	err = ccs_configuration_space_samples(
		configuration_space, NULL, num_configurations, configurations);
	if (err)
		return err;
	if (num_configurations_ret)
		*num_configurations_ret = num_configurations;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
tuner_last_tell(
	ccs_features_tuner_t       tuner,
	size_t                     num_evaluations,
	ccs_features_evaluation_t *evaluations)
{
	if (!num_evaluations)
		return CCS_RESULT_SUCCESS;
	ccs_result_t  err;
	tuner_last_t *tuner_data;
	err = ccs_user_defined_features_tuner_get_tuner_data(
		tuner, (void **)&tuner_data);
	if (err)
		return err;
	err = ccs_retain_object(evaluations[num_evaluations - 1]);
	if (err)
		return err;
	if (tuner_data->last_eval)
		ccs_release_object(tuner_data->last_eval);
	tuner_data->last_eval = evaluations[num_evaluations - 1];
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
tuner_last_get_optima(
	ccs_features_tuner_t       tuner,
	ccs_features_t             features,
	size_t                     num_evaluations,
	ccs_features_evaluation_t *evaluations,
	size_t                    *num_evaluations_ret)
{
	size_t count = 0;
	if (evaluations) {
		ccs_result_t  err;
		tuner_last_t *tuner_data;
		err = ccs_user_defined_features_tuner_get_tuner_data(
			tuner, (void **)&tuner_data);
		if (err)
			return err;
		if (features) {
			ccs_features_t feat;
			int            cmp;
			err = ccs_features_evaluation_get_features(
				tuner_data->last_eval, &feat);
			if (err)
				return err;
			err = ccs_features_cmp(features, feat, &cmp);
			if (err)
				return err;
			if (cmp == 0) {
				if (num_evaluations < 1)
					return CCS_RESULT_ERROR_INVALID_VALUE;
				count          = 1;
				evaluations[0] = tuner_data->last_eval;
			}
		} else {
			if (num_evaluations < 1)
				return CCS_RESULT_ERROR_INVALID_VALUE;
			count          = 1;
			evaluations[0] = tuner_data->last_eval;
		}
		for (size_t i = count; i < num_evaluations; i++)
			evaluations[i] = NULL;
	}
	if (num_evaluations_ret)
		*num_evaluations_ret = count;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
tuner_last_get_history(
	ccs_features_tuner_t       tuner,
	ccs_features_t             features,
	size_t                     num_evaluations,
	ccs_features_evaluation_t *evaluations,
	size_t                    *num_evaluations_ret)
{
	return tuner_last_get_optima(
		tuner, features, num_evaluations, evaluations,
		num_evaluations_ret);
}

ccs_user_defined_features_tuner_vector_t tuner_last_vector = {
	&tuner_last_del,
	&tuner_last_ask,
	&tuner_last_tell,
	&tuner_last_get_optima,
	&tuner_last_get_history,
	NULL,
	NULL,
	NULL};

void
test(void)
{
	ccs_parameter_t           parameter1, parameter2;
	ccs_parameter_t           parameters[2];
	ccs_parameter_t           parameter3;
	ccs_parameter_t           feature;
	ccs_configuration_space_t cspace;
	ccs_features_space_t      fspace;
	ccs_objective_space_t     ospace;
	ccs_expression_t          expression;
	ccs_objective_type_t      otype;
	ccs_features_tuner_t      tuner, tuner_copy;
	ccs_result_t              err;
	ccs_features_t            features_on, features_off;
	tuner_last_t             *tuner_data;
	ccs_datum_t knobs_values[2] = {ccs_string("on"), ccs_string("off")};
	ccs_datum_t d;
	char       *buff;
	size_t      buff_size;
	ccs_map_t   map;

	parameters[0] = parameter1 = create_numerical("x", -5.0, 5.0);
	parameters[1] = parameter2 = create_numerical("y", -5.0, 5.0);

	err                        = ccs_create_configuration_space(
                "2dplane", 2, parameters, NULL, 0, NULL, &cspace);
	assert(err == CCS_RESULT_SUCCESS);

	parameter3 = create_numerical("z", -CCS_INFINITY, CCS_INFINITY);
	err        = ccs_create_variable(parameter3, &expression);
	assert(err == CCS_RESULT_SUCCESS);
	otype = CCS_OBJECTIVE_TYPE_MINIMIZE;

	err   = ccs_create_objective_space(
                "height", 1, &parameter3, 1, &expression, &otype, &ospace);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_categorical_parameter(
		"red knob", 2, knobs_values, 0, &feature);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_features_space("knobs", 1, &feature, &fspace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_features(fspace, 1, knobs_values, &features_on);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_features(fspace, 1, knobs_values + 1, &features_off);
	assert(err == CCS_RESULT_SUCCESS);

	tuner_data = (tuner_last_t *)calloc(1, sizeof(tuner_last_t));
	assert(tuner_data);

	err = ccs_create_user_defined_features_tuner(
		"problem", cspace, fspace, ospace, &tuner_last_vector,
		tuner_data, &tuner);
	assert(err == CCS_RESULT_SUCCESS);

	ccs_features_evaluation_t last_evaluation;
	for (size_t i = 0; i < 50; i++) {
		ccs_datum_t               values[2], res;
		ccs_configuration_t       configuration;
		ccs_features_evaluation_t evaluation;
		err = ccs_features_tuner_ask(
			tuner, features_on, 1, &configuration, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_configuration_get_values(
			configuration, 2, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		res = ccs_float(
			(values[0].value.f - 1) * (values[0].value.f - 1) +
			(values[1].value.f - 2) * (values[1].value.f - 2));
		err = ccs_create_features_evaluation(
			ospace, configuration, features_on, CCS_RESULT_SUCCESS,
			1, &res, &evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_features_tuner_tell(tuner, 1, &evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		last_evaluation = evaluation;
		err             = ccs_release_object(configuration);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(evaluation);
		assert(err == CCS_RESULT_SUCCESS);
	}

	for (size_t i = 0; i < 50; i++) {
		ccs_datum_t               values[2], res;
		ccs_configuration_t       configuration;
		ccs_features_evaluation_t evaluation;
		err = ccs_features_tuner_ask(
			tuner, features_off, 1, &configuration, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_configuration_get_values(
			configuration, 2, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		res = ccs_float(
			(values[0].value.f - 1) * (values[0].value.f - 1) +
			(values[1].value.f - 2) * (values[1].value.f - 2));
		err = ccs_create_features_evaluation(
			ospace, configuration, features_off, CCS_RESULT_SUCCESS,
			1, &res, &evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_features_tuner_tell(tuner, 1, &evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		last_evaluation = evaluation;
		err             = ccs_release_object(configuration);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(evaluation);
		assert(err == CCS_RESULT_SUCCESS);
	}

	size_t                    count;
	ccs_features_evaluation_t history[100];
	err = ccs_features_tuner_get_history(tuner, NULL, 100, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 1);

	ccs_features_evaluation_t evaluation;
	err = ccs_features_tuner_get_optima(
		tuner, NULL, 1, &evaluation, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 1);
	assert(last_evaluation == evaluation);

	err = ccs_features_tuner_get_optima(
		tuner, features_on, 1, &evaluation, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 0);

	err = ccs_features_tuner_get_optima(
		tuner, features_off, 1, &evaluation, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 1);

	/* Test (de)serialization */
	err = ccs_create_map(&map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_object_serialize(
		tuner, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		tuner, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	tuner_data = (tuner_last_t *)calloc(1, sizeof(tuner_last_t));
	assert(tuner_data);

	err = ccs_object_deserialize(
		(ccs_object_t *)&tuner_copy, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_VECTOR, &tuner_last_vector,
		CCS_DESERIALIZE_OPTION_DATA, tuner_data,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_features_tuner_get_history(
		tuner_copy, NULL, 100, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 1);

	err = ccs_features_tuner_get_optima(
		tuner_copy, NULL, 1, &evaluation, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 1);

	err = ccs_map_get(map, ccs_object((ccs_object_t)tuner), &d);
	assert(err == CCS_RESULT_SUCCESS);
	assert(d.type == CCS_DATA_TYPE_OBJECT);
	assert(d.value.o == (ccs_object_t)tuner_copy);

	free(buff);
	err = ccs_release_object(map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(tuner_copy);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter2);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter3);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(feature);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(features_on);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(features_off);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(cspace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(ospace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(fspace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(tuner);
	assert(err == CCS_RESULT_SUCCESS);
}

int
main(void)
{
	ccs_init();
	test();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
