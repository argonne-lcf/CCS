#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>
#include "test_utils.h"

struct tuner_last_s {
	ccs_evaluation_t last_eval;
};
typedef struct tuner_last_s tuner_last_t;

ccs_result_t
tuner_last_del(ccs_tuner_t tuner)
{
	tuner_last_t *tuner_data;
	ccs_result_t  err;
	err = ccs_user_defined_tuner_get_tuner_data(
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
	ccs_tuner_t                 tuner,
	ccs_features_t              features,
	size_t                      num_configurations,
	ccs_search_configuration_t *configurations,
	size_t                     *num_configurations_ret)
{
	(void)features;
	if (!configurations) {
		*num_configurations_ret = 1;
		return CCS_RESULT_SUCCESS;
	}
	ccs_result_t              err;
	ccs_configuration_space_t configuration_space;
	err = ccs_tuner_get_search_space(
		tuner, (ccs_search_space_t *)&configuration_space);
	if (err)
		return err;
	err = ccs_configuration_space_samples(
		configuration_space, NULL, features, NULL, num_configurations,
		(ccs_configuration_t *)configurations);
	if (err)
		return err;
	if (num_configurations_ret)
		*num_configurations_ret = num_configurations;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
tuner_last_tell(
	ccs_tuner_t       tuner,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations)
{
	if (!num_evaluations)
		return CCS_RESULT_SUCCESS;
	ccs_result_t  err;
	tuner_last_t *tuner_data;
	err = ccs_user_defined_tuner_get_tuner_data(
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
	ccs_tuner_t       tuner,
	ccs_features_t    features,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations,
	size_t           *num_evaluations_ret)
{
	size_t count = 0;
	if (evaluations) {
		ccs_result_t  err;
		tuner_last_t *tuner_data;
		err = ccs_user_defined_tuner_get_tuner_data(
			tuner, (void **)&tuner_data);
		if (err)
			return err;
		if (features) {
			ccs_features_t feat;
			int            cmp;
			err = ccs_evaluation_get_features(
				tuner_data->last_eval, &feat);
			if (err)
				return err;
			err = ccs_binding_cmp(
				(ccs_binding_t)features, (ccs_binding_t)feat,
				&cmp);
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
	ccs_tuner_t       tuner,
	ccs_features_t    features,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations,
	size_t           *num_evaluations_ret)
{
	return tuner_last_get_optima(
		tuner, features, num_evaluations, evaluations,
		num_evaluations_ret);
}

ccs_user_defined_tuner_vector_t tuner_last_vector = {
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
	ccs_configuration_space_t cspace;
	ccs_feature_space_t       fspace;
	ccs_objective_space_t     ospace;
	ccs_tuner_t               tuner, tuner_copy;
	ccs_result_t              err;
	ccs_features_t            features_on, features_off;
	tuner_last_t             *tuner_data;
	ccs_datum_t               d;
	char                     *buff;
	size_t                    buff_size;
	ccs_map_t                 map;

	fspace     = create_knobs(&features_on, &features_off);
	cspace     = create_2d_plane(fspace);
	ospace     = create_height_objective(cspace);

	tuner_data = (tuner_last_t *)calloc(1, sizeof(tuner_last_t));
	assert(tuner_data);

	err = ccs_create_user_defined_tuner(
		"problem", ospace, &tuner_last_vector, tuner_data, &tuner);
	assert(err == CCS_RESULT_SUCCESS);

	ccs_evaluation_t last_evaluation;
	for (size_t i = 0; i < 50; i++) {
		ccs_datum_t                values[2], res;
		ccs_search_configuration_t configuration;
		ccs_evaluation_t           evaluation;
		err = ccs_tuner_ask(
			tuner, features_on, 1, &configuration, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_binding_get_values(
			(ccs_binding_t)configuration, 2, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		res = ccs_float(
			(values[0].value.f - 1) * (values[0].value.f - 1) +
			(values[1].value.f - 2) * (values[1].value.f - 2));
		err = ccs_create_evaluation(
			ospace, configuration, CCS_RESULT_SUCCESS, 1, &res,
			&evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_tuner_tell(tuner, 1, &evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		last_evaluation = evaluation;
		err             = ccs_release_object(configuration);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(evaluation);
		assert(err == CCS_RESULT_SUCCESS);
	}

	for (size_t i = 0; i < 50; i++) {
		ccs_datum_t                values[2], res;
		ccs_search_configuration_t configuration;
		ccs_evaluation_t           evaluation;
		err = ccs_tuner_ask(
			tuner, features_off, 1, &configuration, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_binding_get_values(
			(ccs_binding_t)configuration, 2, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		res = ccs_float(
			(values[0].value.f - 1) * (values[0].value.f - 1) +
			(values[1].value.f - 2) * (values[1].value.f - 2));
		err = ccs_create_evaluation(
			ospace, configuration, CCS_RESULT_SUCCESS, 1, &res,
			&evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_tuner_tell(tuner, 1, &evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		last_evaluation = evaluation;
		err             = ccs_release_object(configuration);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(evaluation);
		assert(err == CCS_RESULT_SUCCESS);
	}

	size_t           count;
	ccs_evaluation_t history[100];
	err = ccs_tuner_get_history(tuner, NULL, 100, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 1);

	ccs_evaluation_t evaluation;
	err = ccs_tuner_get_optima(tuner, NULL, 1, &evaluation, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 1);
	assert(last_evaluation == evaluation);

	err = ccs_tuner_get_optima(tuner, features_on, 1, &evaluation, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 0);

	err = ccs_tuner_get_optima(tuner, features_off, 1, &evaluation, &count);
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

	err = ccs_tuner_get_history(tuner_copy, NULL, 100, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 1);

	err = ccs_tuner_get_optima(tuner_copy, NULL, 1, &evaluation, &count);
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
