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

	err = ccs_create_random_features_tuner(
		"problem", cspace, fspace, ospace, &tuner);
	assert(err == CCS_RESULT_SUCCESS);

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
		err = ccs_release_object(configuration);
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
		err = ccs_release_object(configuration);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(evaluation);
		assert(err == CCS_RESULT_SUCCESS);
	}

	size_t                    count;
	ccs_features_evaluation_t history[100];
	ccs_features_evaluation_t evaluation;
	ccs_datum_t               min_on  = ccs_float(INFINITY);
	ccs_datum_t               min_off = ccs_float(INFINITY);
	err = ccs_features_tuner_get_history(tuner, NULL, 0, NULL, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 100);

	err = ccs_features_tuner_get_optima(tuner, NULL, 0, NULL, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 2);

	err = ccs_features_tuner_get_history(
		tuner, features_on, 50, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 50);

	for (size_t i = 0; i < 50; i++) {
		ccs_datum_t res;
		err = ccs_features_evaluation_get_objective_value(
			history[i], 0, &res);
		assert(err == CCS_RESULT_SUCCESS);
		if (res.value.f < min_on.value.f)
			min_on.value.f = res.value.f;
	}

	err = ccs_features_tuner_get_history(
		tuner, features_off, 50, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 50);

	for (size_t i = 0; i < 50; i++) {
		ccs_datum_t res;
		err = ccs_features_evaluation_get_objective_value(
			history[i], 0, &res);
		assert(err == CCS_RESULT_SUCCESS);
		if (res.value.f < min_off.value.f)
			min_off.value.f = res.value.f;
	}

	ccs_datum_t res;
	err = ccs_features_tuner_get_optima(
		tuner, features_on, 1, &evaluation, NULL);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_features_evaluation_get_objective_value(evaluation, 0, &res);
	assert(res.value.f == min_on.value.f);

	err = ccs_features_tuner_get_optima(
		tuner, features_off, 1, &evaluation, NULL);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_features_evaluation_get_objective_value(evaluation, 0, &res);
	assert(res.value.f == min_off.value.f);

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

	err = ccs_map_get(map, ccs_object((ccs_object_t)tuner), &d);
	assert(err == CCS_RESULT_SUCCESS);
	assert(d.type == CCS_DATA_TYPE_OBJECT);
	assert(d.value.o == (ccs_object_t)tuner_copy);

	err = ccs_features_tuner_get_history(tuner_copy, NULL, 0, NULL, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 100);

	err = ccs_features_tuner_get_optima(tuner_copy, NULL, 0, NULL, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 2);

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

void
test_evaluation_deserialize(void)
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
	ccs_result_t              err;
	ccs_configuration_t       configuration;
	ccs_features_t            features_on;
	ccs_datum_t knobs_values[2] = {ccs_string("on"), ccs_string("off")};
	ccs_features_evaluation_t evaluation_ref, evaluation;
	ccs_datum_t               res, d;
	char                     *buff;
	size_t                    buff_size;
	ccs_map_t                 map;
	int                       cmp;

	parameters[0] = parameter1 = create_numerical("x", -5.0, 5.0);
	parameters[1] = parameter2 = create_numerical("y", -5.0, 5.0);

	err                        = ccs_create_configuration_space(
                "2dplane", 2, parameters, NULL, 0, NULL, &cspace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_configuration_space_sample(
		cspace, NULL, NULL, &configuration);
	assert(err == CCS_RESULT_SUCCESS);

	parameter3 = create_numerical("z", -CCS_INFINITY, CCS_INFINITY);
	err        = ccs_create_variable(parameter3, &expression);
	assert(err == CCS_RESULT_SUCCESS);
	otype = CCS_OBJECTIVE_TYPE_MINIMIZE;

	err   = ccs_create_categorical_parameter(
                "red knob", 2, knobs_values, 0, &feature);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_features_space("knobs", 1, &feature, &fspace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_features(fspace, 1, knobs_values, &features_on);
	err = ccs_create_objective_space(
		"height", 1, &parameter3, 1, &expression, &otype, &ospace);
	assert(err == CCS_RESULT_SUCCESS);

	res = ccs_float(1.5);
	err = ccs_create_features_evaluation(
		ospace, configuration, features_on, CCS_RESULT_SUCCESS, 1, &res,
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

	d = ccs_object(fspace);
	d.flags |= CCS_DATUM_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(fspace));
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

	err = ccs_features_evaluation_cmp(evaluation_ref, evaluation, &cmp);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!cmp);

	free(buff);
	err = ccs_release_object(feature);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(features_on);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(evaluation_ref);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(evaluation);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(configuration);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter2);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter3);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(cspace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(fspace);
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
