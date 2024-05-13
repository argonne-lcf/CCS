#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>
#include "test_utils.h"

void
test(void)
{
	ccs_configuration_space_t cspace;
	ccs_objective_space_t     ospace;
	ccs_tuner_t               tuner, tuner_copy;
	ccs_result_t              err;
	ccs_datum_t               d;
	char                     *buff;
	size_t                    buff_size;
	ccs_map_t                 map;

	cspace = create_2d_plane(NULL);
	ospace = create_height_objective(cspace);

	err    = ccs_create_random_tuner("problem", ospace, &tuner);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t                values[2], res;
		ccs_search_configuration_t configuration;
		ccs_evaluation_t           evaluation;
		err = ccs_tuner_ask(tuner, NULL, 1, &configuration, NULL);
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
		err = ccs_release_object(configuration);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(evaluation);
		assert(err == CCS_RESULT_SUCCESS);
	}

	size_t           count;
	ccs_evaluation_t history[100];
	ccs_datum_t      min = ccs_float(INFINITY);
	err = ccs_tuner_get_history(tuner, NULL, 100, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 100);

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t res;
		err = ccs_evaluation_get_objective_value(history[i], 0, &res);
		assert(err == CCS_RESULT_SUCCESS);
		if (res.value.f < min.value.f)
			min.value.f = res.value.f;
	}

	ccs_evaluation_t evaluation;
	ccs_datum_t      res;
	err = ccs_tuner_get_optima(tuner, NULL, 1, &evaluation, NULL);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_evaluation_get_objective_value(evaluation, 0, &res);
	assert(res.value.f == min.value.f);

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

	err = ccs_object_deserialize(
		(ccs_object_t *)&tuner_copy, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_tuner_get_history(tuner_copy, NULL, 100, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 100);

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
	err = ccs_release_object(cspace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(ospace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(tuner);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_evaluation_deserialize(void)
{
	ccs_configuration_space_t  cspace;
	ccs_objective_space_t      ospace;
	ccs_result_t               err;
	ccs_search_configuration_t configuration;
	ccs_evaluation_t           evaluation_ref, evaluation;
	ccs_datum_t                res, d;
	char                      *buff;
	size_t                     buff_size;
	ccs_map_t                  map;
	int                        cmp;

	cspace = create_2d_plane(NULL);
	ospace = create_height_objective(cspace);

	err    = ccs_configuration_space_sample(
                cspace, NULL, NULL, NULL,
                (ccs_configuration_t *)&configuration);
	assert(err == CCS_RESULT_SUCCESS);

	res = ccs_float(1.5);
	err = ccs_create_evaluation(
		ospace, configuration, CCS_RESULT_SUCCESS, 1, &res,
		&evaluation_ref);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_map(&map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_object_serialize(
		evaluation_ref, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		evaluation_ref, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&evaluation, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_HANDLE);

	d = ccs_object(ospace);
	d.flags |= CCS_DATUM_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(ospace));
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&evaluation, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_HANDLE);

	d = ccs_object(cspace);
	d.flags |= CCS_DATUM_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(cspace));
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&evaluation, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_binding_cmp(
		(ccs_binding_t)evaluation_ref, (ccs_binding_t)evaluation, &cmp);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!cmp);

	free(buff);
	err = ccs_release_object(map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(evaluation_ref);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(evaluation);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(configuration);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(cspace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(ospace);
	assert(err == CCS_RESULT_SUCCESS);
}

int
main(void)
{
	ccs_init();
	test();
	test_evaluation_deserialize();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
