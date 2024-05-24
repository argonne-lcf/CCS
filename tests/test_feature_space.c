#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

double d = -2.0;

ccs_parameter_t
create_dummy_parameter(const char *name)
{
	ccs_parameter_t parameter;
	ccs_result_t    err;
	err = ccs_create_numerical_parameter(
		name, CCS_NUMERIC_TYPE_FLOAT, CCSF(-5.0), CCSF(5.0), CCSF(0.0),
		CCSF(d), &parameter);
	d += 1.0;
	if (d >= 5.0)
		d = -5.0;
	assert(err == CCS_RESULT_SUCCESS);
	return parameter;
}

void
check_features(
	ccs_feature_space_t feature_space,
	size_t              sz,
	ccs_parameter_t    *parameters)
{
	ccs_parameter_t  parameter;
	ccs_result_t     err;
	size_t           sz_ret;
	size_t           index;
	ccs_parameter_t *parameters_ret =
		(ccs_parameter_t *)malloc(sizeof(ccs_parameter_t) * (sz + 1));
	const char *name;

	err = ccs_context_get_parameters(
		(ccs_context_t)feature_space, 0, NULL, &sz_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(sz_ret == sz);

	for (size_t i = 0; i < sz; i++) {
		err = ccs_context_get_parameter(
			(ccs_context_t)feature_space, i, &parameter);
		assert(err == CCS_RESULT_SUCCESS);
		assert(parameter == parameters[i]);
		err = ccs_context_get_parameter_index(
			(ccs_context_t)feature_space, parameter, NULL, &index);
		assert(err == CCS_RESULT_SUCCESS);
		assert(index == i);
	}
	err = ccs_context_get_parameters(
		(ccs_context_t)feature_space, sz + 1, parameters_ret, &sz_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(sz_ret == sz);
	for (size_t i = 0; i < sz; i++)
		assert(parameters_ret[i] == parameters[i]);
	assert(parameters_ret[sz] == NULL);

	for (size_t i = 0; i < sz; i++) {
		err = ccs_parameter_get_name(parameters[i], &name);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_context_get_parameter_by_name(
			(ccs_context_t)feature_space, name, &parameter);
		assert(err == CCS_RESULT_SUCCESS);
		assert(parameter == parameters[i]);
	}

	free(parameters_ret);
}

void
test_create(void)
{
	ccs_parameter_t     parameters[3];
	ccs_parameter_t     parameter3;
	ccs_feature_space_t feature_space;
	ccs_result_t        err;
	ccs_object_type_t   type;
	const char         *name;

	parameters[0] = create_dummy_parameter("param1");
	parameters[1] = create_dummy_parameter("param2");
	parameter3    = create_dummy_parameter("param3");
	parameters[2] = parameters[0];

	err           = ccs_create_feature_space(
                "my_feature_space", 3, NULL, &feature_space);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);

	err = ccs_create_feature_space(
		"my_feature_space", 3, parameters, &feature_space);
	assert(err == CCS_RESULT_ERROR_INVALID_PARAMETER);

	parameters[2] = parameter3;

	err           = ccs_create_feature_space(
                "my_feature_space", 3, parameters, &feature_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_get_type(feature_space, &type);
	assert(err == CCS_RESULT_SUCCESS);
	assert(type == CCS_OBJECT_TYPE_FEATURE_SPACE);

	err = ccs_context_get_name((ccs_context_t)feature_space, &name);
	assert(err == CCS_RESULT_SUCCESS);
	assert(strcmp(name, "my_feature_space") == 0);

	check_features(feature_space, 3, parameters);

	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
	err = ccs_release_object(feature_space);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_features(void)
{
	ccs_parameter_t     parameters[3];
	ccs_feature_space_t feature_space, feature_space_ret;
	ccs_datum_t         values[3] = {
                ccs_float(-1.0), ccs_float(0.0), ccs_float(1.0)};
	ccs_datum_t    values_ret[3];
	ccs_features_t features1, features2;
	ccs_result_t   err;
	ccs_datum_t    datum;
	size_t         num_values_ret;
	int            cmp;
	ccs_bool_t     check;

	parameters[0] = create_dummy_parameter("param1");
	parameters[1] = create_dummy_parameter("param2");
	parameters[2] = create_dummy_parameter("param3");

	err           = ccs_create_feature_space(
                "my_feature_space", 3, parameters, &feature_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_features(feature_space, 3, values, &features1);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_features_get_feature_space(features1, &feature_space_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(feature_space == feature_space_ret);

	for (size_t i = 0; i < 3; i++) {
		err = ccs_binding_get_value(
			(ccs_binding_t)features1, i, &datum);
		assert(err == CCS_RESULT_SUCCESS);
		assert(values[i].type == datum.type);
		assert(values[i].value.f == datum.value.f);
	}

	err = ccs_binding_get_values(
		(ccs_binding_t)features1, 3, values_ret, &num_values_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(3 == num_values_ret);
	for (size_t i = 0; i < 3; i++) {
		assert(values[i].type == values_ret[i].type);
		assert(values[i].value.f == values_ret[i].value.f);
	}

	err = ccs_binding_get_value_by_name(
		(ccs_binding_t)features1, "param2", NULL, &datum);
	assert(err == CCS_RESULT_SUCCESS);
	assert(values[1].type == datum.type);
	assert(values[1].value.f == datum.value.f);

	err = ccs_features_check(features1, &check);
	assert(err == CCS_RESULT_SUCCESS);
	assert(check);

	err = ccs_feature_space_check_features(
		feature_space, features1, &check);
	assert(err == CCS_RESULT_SUCCESS);
	assert(check);

	err = ccs_create_features(feature_space, 3, values, &features2);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_binding_cmp(
		(ccs_binding_t)features1, (ccs_binding_t)features2, &cmp);
	assert(err == CCS_RESULT_SUCCESS);
	assert(0 == cmp);
	err = ccs_release_object(features2);
	assert(err == CCS_RESULT_SUCCESS);

	values[1] = ccs_float(0.5);
	err       = ccs_create_features(feature_space, 3, values, &features2);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_binding_cmp(
		(ccs_binding_t)features1, (ccs_binding_t)features2, &cmp);
	assert(err == CCS_RESULT_SUCCESS);
	assert(0 > cmp);
	err = ccs_release_object(features2);
	assert(err == CCS_RESULT_SUCCESS);

	values[1] = ccs_float(10.0);
	err       = ccs_create_features(feature_space, 3, values, &features2);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_features_check(features2, &check);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!check);

	err = ccs_feature_space_check_features(
		feature_space, features2, &check);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!check);

	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
	err = ccs_release_object(feature_space);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(features1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(features2);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_deserialize(void)
{
	ccs_parameter_t     parameters[3], parameters_new[3];
	ccs_feature_space_t feature_space, feature_space_ref;
	ccs_map_t           map;
	ccs_result_t        err;
	char               *buff;
	size_t              buff_size;
	ccs_datum_t         d;

	parameters[0] = create_dummy_parameter("param1");
	parameters[1] = create_dummy_parameter("param2");
	parameters[2] = create_dummy_parameter("param3");

	err           = ccs_create_feature_space(
                "my_config_space", 3, parameters, &feature_space);
	assert(err == CCS_RESULT_SUCCESS);

	check_features(feature_space, 3, parameters);

	err = ccs_object_serialize(
		feature_space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		feature_space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
	feature_space_ref = feature_space;
	err               = ccs_release_object(feature_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&feature_space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_context_get_parameter_by_name(
		(ccs_context_t)feature_space, "param1", &parameters_new[0]);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_context_get_parameter_by_name(
		(ccs_context_t)feature_space, "param2", &parameters_new[1]);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_context_get_parameter_by_name(
		(ccs_context_t)feature_space, "param3", &parameters_new[2]);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(feature_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_map(&map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_object_deserialize(
		(ccs_object_t *)&feature_space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_context_get_parameter_by_name(
		(ccs_context_t)feature_space, "param1", &parameters_new[0]);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_context_get_parameter_by_name(
		(ccs_context_t)feature_space, "param2", &parameters_new[1]);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_context_get_parameter_by_name(
		(ccs_context_t)feature_space, "param3", &parameters_new[2]);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_map_get(map, ccs_object(feature_space_ref), &d);
	assert(err == CCS_RESULT_SUCCESS);
	assert(d.type == CCS_DATA_TYPE_OBJECT);
	assert(d.value.o == feature_space);

	err = ccs_release_object(map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(feature_space);
	assert(err == CCS_RESULT_SUCCESS);
	free(buff);
}

void
test_features_deserialize(void)
{
	ccs_parameter_t     parameters[3];
	ccs_feature_space_t feature_space;
	ccs_features_t      features, features_ref;
	ccs_datum_t         values[3] = {
                ccs_float(-1.0), ccs_float(0.0), ccs_float(1.0)};
	char        *buff;
	size_t       buff_size;
	ccs_map_t    map;
	ccs_datum_t  d;
	ccs_result_t err;
	int          cmp;

	parameters[0] = create_dummy_parameter("param1");
	parameters[1] = create_dummy_parameter("param2");
	parameters[2] = create_dummy_parameter("param3");

	err           = ccs_create_feature_space(
                "my_config_space", 3, parameters, &feature_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_features(feature_space, 3, values, &features_ref);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_map(&map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_object_serialize(
		features_ref, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		features_ref, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&features, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_HANDLE);

	d = ccs_object(feature_space);
	d.flags |= CCS_DATUM_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(feature_space));
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&features, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_binding_cmp(
		(ccs_binding_t)features_ref, (ccs_binding_t)features, &cmp);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!cmp);

	free(buff);
	err = ccs_release_object(feature_space);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(features_ref);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(features);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(map);
	assert(err == CCS_RESULT_SUCCESS);
	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
}

int
main(void)
{
	ccs_init();
	test_create();
	test_features();
	test_deserialize();
	test_features_deserialize();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
