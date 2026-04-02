#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>
#include "test_utils.h"

void
test(ccs_serialize_format_t format)
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
		tuner, format, CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		tuner, format, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&tuner_copy, format,
		CCS_DESERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_MAP_HANDLES, CCS_DESERIALIZE_OPTION_END);
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
test_objective_space_deserialize(void)
{
	ccs_configuration_space_t cspace;
	ccs_objective_space_t     ospace, ospace_copy;
	ccs_parameter_t           param;
	ccs_expression_t          expression;
	ccs_objective_type_t      otype;
	ccs_result_t              err;
	ccs_serialize_format_t    formats[] = {
                CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_FORMAT_JSON};
	size_t num_formats = sizeof(formats) / sizeof(formats[0]);

	cspace             = create_2d_plane(NULL);

	/* Use finite bounds so JSON roundtrip works (cJSON cannot
	 * represent Infinity). */
	param              = create_numerical("z", -1e6, 1e6);
	err                = ccs_create_variable(param, &expression);
	assert(err == CCS_RESULT_SUCCESS);
	otype = CCS_OBJECTIVE_TYPE_MINIMIZE;
	err   = ccs_create_objective_space(
                "height", (ccs_search_space_t)cspace, 1, &param, 1, &expression,
                &otype, &ospace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(param);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t f = 0; f < num_formats; f++) {
		test_serialize_deserialize(
			ospace, formats[f], (ccs_object_t *)&ospace_copy);
		err = ccs_release_object(ospace_copy);
		assert(err == CCS_RESULT_SUCCESS);
	}

	err = ccs_release_object(cspace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(ospace);
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
	ccs_serialize_format_t     formats[] = {
                CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_FORMAT_JSON};
	size_t num_formats = sizeof(formats) / sizeof(formats[0]);

	cspace             = create_2d_plane(NULL);
	ospace             = create_height_objective(cspace);

	err                = ccs_configuration_space_sample(
                cspace, NULL, NULL, NULL,
                (ccs_configuration_t *)&configuration);
	assert(err == CCS_RESULT_SUCCESS);

	res = ccs_float(1.5);
	err = ccs_create_evaluation(
		ospace, configuration, CCS_RESULT_SUCCESS, 1, &res,
		&evaluation_ref);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t f = 0; f < num_formats; f++) {
		err = ccs_create_map(&map);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_object_serialize(
			evaluation_ref, formats[f],
			CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
			CCS_SERIALIZE_OPTION_END);
		assert(err == CCS_RESULT_SUCCESS);
		buff = (char *)malloc(buff_size);
		assert(buff);

		err = ccs_object_serialize(
			evaluation_ref, formats[f],
			CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
			CCS_SERIALIZE_OPTION_END);
		assert(err == CCS_RESULT_SUCCESS);

		err = ccs_object_deserialize(
			(ccs_object_t *)&evaluation, formats[f],
			CCS_DESERIALIZE_OPERATION_MEMORY, buff_size, buff,
			CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
			CCS_DESERIALIZE_OPTION_END);
		assert(err == CCS_RESULT_ERROR_INVALID_HANDLE);
		ccs_clear_thread_error();

		d = ccs_object(ospace);
		d.flags |= CCS_DATUM_FLAG_ID;
		err = ccs_map_set(map, d, ccs_object(ospace));
		assert(err == CCS_RESULT_SUCCESS);

		err = ccs_object_deserialize(
			(ccs_object_t *)&evaluation, formats[f],
			CCS_DESERIALIZE_OPERATION_MEMORY, buff_size, buff,
			CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
			CCS_DESERIALIZE_OPTION_END);
		assert(err == CCS_RESULT_ERROR_INVALID_HANDLE);
		ccs_clear_thread_error();

		d = ccs_object(cspace);
		d.flags |= CCS_DATUM_FLAG_ID;
		err = ccs_map_set(map, d, ccs_object(cspace));
		assert(err == CCS_RESULT_SUCCESS);

		err = ccs_object_deserialize(
			(ccs_object_t *)&evaluation, formats[f],
			CCS_DESERIALIZE_OPERATION_MEMORY, buff_size, buff,
			CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
			CCS_DESERIALIZE_OPTION_END);
		assert(err == CCS_RESULT_SUCCESS);

		err = ccs_binding_cmp(
			(ccs_binding_t)evaluation_ref,
			(ccs_binding_t)evaluation, &cmp);
		assert(err == CCS_RESULT_SUCCESS);
		assert(!cmp);

		free(buff);
		err = ccs_release_object(map);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(evaluation);
		assert(err == CCS_RESULT_SUCCESS);
	}

	err = ccs_release_object(evaluation_ref);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(configuration);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(cspace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(ospace);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_evaluation_hash_cmp_compare(void)
{
	ccs_configuration_space_t  cspace;
	ccs_objective_space_t      ospace;
	ccs_tuner_t                tuner;
	ccs_result_t               err;
	ccs_search_configuration_t config1, config2;
	ccs_evaluation_t           eval1, eval2;
	ccs_datum_t                res1, res2;
	ccs_hash_t                 hash1, hash2;
	ccs_comparison_t           comparison;
	int                        cmp;

	cspace = create_2d_plane(NULL);
	ospace = create_height_objective(cspace);

	err    = ccs_create_random_tuner("test", ospace, &tuner);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_tuner_ask(tuner, NULL, 1, &config1, NULL);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_tuner_ask(tuner, NULL, 1, &config2, NULL);
	assert(err == CCS_RESULT_SUCCESS);

	res1 = ccs_float(1.0);
	res2 = ccs_float(2.0);

	err  = ccs_create_evaluation(
                ospace, config1, CCS_RESULT_SUCCESS, 1, &res1, &eval1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_evaluation(
		ospace, config2, CCS_RESULT_SUCCESS, 1, &res2, &eval2);
	assert(err == CCS_RESULT_SUCCESS);

	/* Hash */
	err = ccs_binding_hash((ccs_binding_t)eval1, &hash1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_binding_hash((ccs_binding_t)eval2, &hash2);
	assert(err == CCS_RESULT_SUCCESS);

	/* Cmp with itself should be 0 */
	err = ccs_binding_cmp((ccs_binding_t)eval1, (ccs_binding_t)eval1, &cmp);
	assert(err == CCS_RESULT_SUCCESS);
	assert(cmp == 0);

	/* Compare evaluations — eval1 (1.0) is better than eval2 (2.0)
	 * for a minimization objective */
	err = ccs_evaluation_compare(eval1, eval2, &comparison);
	assert(err == CCS_RESULT_SUCCESS);
	assert(comparison == CCS_COMPARISON_BETTER);

	/* get_objective_values with exact count */
	{
		ccs_datum_t values[1];
		err = ccs_evaluation_get_objective_values(
			eval1, 1, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		assert(values[0].value.f == 1.0);
	}

	/* get_objective_values with oversized buffer should succeed */
	{
		ccs_datum_t values[2];
		err = ccs_evaluation_get_objective_values(
			eval1, 2, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		assert(values[0].value.f == 1.0);
	}

	ccs_release_object(eval2);
	ccs_release_object(eval1);
	ccs_release_object(config2);
	ccs_release_object(config1);
	ccs_release_object(tuner);
	ccs_release_object(ospace);
	ccs_release_object(cspace);
}

int
main(void)
{
	ccs_init();
	test(CCS_SERIALIZE_FORMAT_BINARY);
	test(CCS_SERIALIZE_FORMAT_JSON);
	test_objective_space_deserialize();
	test_evaluation_deserialize();
	test_evaluation_hash_cmp_compare();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
