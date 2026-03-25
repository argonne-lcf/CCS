#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>
#include "test_utils.h"

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
check_configuration(
	ccs_configuration_space_t configuration_space,
	size_t                    sz,
	ccs_parameter_t          *parameters)
{
	ccs_parameter_t     parameter;
	ccs_configuration_t configuration;
	ccs_result_t        err;
	size_t              sz_ret;
	size_t              index;
	ccs_parameter_t    *parameters_ret =
		(ccs_parameter_t *)malloc(sizeof(ccs_parameter_t) * (sz + 1));
	const char *name;

	err = ccs_context_get_parameters(
		(ccs_context_t)configuration_space, 0, NULL, &sz_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(sz_ret == sz);

	for (size_t i = 0; i < sz; i++) {
		err = ccs_context_get_parameter(
			(ccs_context_t)configuration_space, i, &parameter);
		assert(err == CCS_RESULT_SUCCESS);
		assert(parameter == parameters[i]);
		err = ccs_context_get_parameter_index(
			(ccs_context_t)configuration_space, parameter, NULL,
			&index);
		assert(err == CCS_RESULT_SUCCESS);
		assert(index == i);
	}
	err = ccs_context_get_parameters(
		(ccs_context_t)configuration_space, sz + 1, parameters_ret,
		&sz_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(sz_ret == sz);
	for (size_t i = 0; i < sz; i++)
		assert(parameters_ret[i] == parameters[i]);
	assert(parameters_ret[sz] == NULL);

	for (size_t i = 0; i < sz; i++) {
		err = ccs_parameter_get_name(parameters[i], &name);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_context_get_parameter_by_name(
			(ccs_context_t)configuration_space, name, &parameter);
		assert(err == CCS_RESULT_SUCCESS);
		assert(parameter == parameters[i]);
	}

	err = ccs_configuration_space_get_default_configuration(
		configuration_space, NULL, &configuration);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t i = 0; i < sz; i++) {
		ccs_datum_t datum;
		ccs_datum_t hdatum;
		err = ccs_binding_get_value(
			(ccs_binding_t)configuration, i, &datum);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_parameter_get_default_value(parameters[i], &hdatum);
		assert(err == CCS_RESULT_SUCCESS);
		assert(datum.type == hdatum.type);
		assert(datum.value.f == hdatum.value.f);
	}
	err = ccs_release_object(configuration);
	assert(err == CCS_RESULT_SUCCESS);
	free(parameters_ret);
}

void
test_empty(void)
{
	ccs_configuration_space_t configuration_space;
	ccs_configuration_t       configuration;
	ccs_result_t              err;

	err = ccs_create_configuration_space(
		"my_config_space", 0, NULL, NULL, 0, NULL, NULL, NULL,
		&configuration_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_configuration_space_sample(
		configuration_space, NULL, NULL, NULL, &configuration);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(configuration);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_configuration_space_get_default_configuration(
		configuration_space, NULL, &configuration);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(configuration);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(configuration_space);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_create(void)
{
	ccs_parameter_t           parameters[3];
	ccs_parameter_t           parameter3;
	ccs_configuration_space_t configuration_space;
	ccs_result_t              err;
	ccs_object_type_t         type;
	const char               *name;

	parameters[0] = create_dummy_parameter("param1");
	parameters[1] = create_dummy_parameter("param2");
	parameter3    = create_dummy_parameter("param3");
	parameters[2] = parameters[0];

	err           = ccs_create_configuration_space(
                "my_config_space", 3, NULL, NULL, 0, NULL, NULL, NULL,
                &configuration_space);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);

	err = ccs_create_configuration_space(
		"my_config_space", 3, parameters, NULL, 0, NULL, NULL, NULL,
		&configuration_space);
	assert(err == CCS_RESULT_ERROR_INVALID_PARAMETER);

	parameters[2] = parameter3;

	err           = ccs_create_configuration_space(
                "my_config_space", 3, parameters, NULL, 0, NULL, NULL, NULL,
                &configuration_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_get_type(configuration_space, &type);
	assert(err == CCS_RESULT_SUCCESS);
	assert(type == CCS_OBJECT_TYPE_CONFIGURATION_SPACE);

	err = ccs_context_get_name((ccs_context_t)configuration_space, &name);
	assert(err == CCS_RESULT_SUCCESS);
	assert(strcmp(name, "my_config_space") == 0);

	check_configuration(configuration_space, 3, parameters);

	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
	err = ccs_release_object(configuration_space);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_sample(void)
{
	ccs_parameter_t           parameters[4];
	ccs_configuration_t       configuration;
	ccs_configuration_t       configurations[100];
	ccs_configuration_space_t configuration_space;
	ccs_result_t              err;

	parameters[0] = create_dummy_parameter("param1");
	parameters[1] = create_dummy_parameter("param2");
	parameters[2] = create_dummy_parameter("param3");
	err           = ccs_create_numerical_parameter(
                "param4", CCS_NUMERIC_TYPE_INT, CCSI(-5), CCSI(5), CCSI(0),
                CCSI(0), parameters + 3);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_configuration_space(
		"my_config_space", 4, parameters, NULL, 0, NULL, NULL, NULL,
		&configuration_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_configuration_space_sample(
		configuration_space, NULL, NULL, NULL, &configuration);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_configuration_space_samples(
		configuration_space, NULL, NULL, NULL, 100, configurations);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t i = 0; i < 100; i++) {
		err = ccs_release_object(configurations[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}

	err = ccs_release_object(configuration);
	assert(err == CCS_RESULT_SUCCESS);
	for (size_t i = 0; i < 4; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
	err = ccs_release_object(configuration_space);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_configuration_deserialize(void)
{
	ccs_parameter_t           parameters[3];
	ccs_configuration_space_t configuration_space;
	ccs_configuration_t       configuration, configuration_ref;
	char                     *buff;
	size_t                    buff_size;
	ccs_map_t                 map;
	ccs_datum_t               d;
	ccs_result_t              err;
	int                       cmp;

	parameters[0] = create_numerical("param1", -5.0, 5.0);
	parameters[1] = create_numerical("param2", -5.0, 5.0);
	parameters[2] = create_numerical("param3", -5.0, 5.0);

	err           = ccs_create_configuration_space(
                "my_config_space", 3, parameters, NULL, 0, NULL, NULL, NULL,
                &configuration_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_configuration_space_sample(
		configuration_space, NULL, NULL, NULL, &configuration_ref);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_map(&map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_object_serialize(
		configuration_ref, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		configuration_ref, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&configuration, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_DESERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_HANDLE);

	d = ccs_object(configuration_space);
	d.flags |= CCS_DATUM_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(configuration_space));
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&configuration, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_DESERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_binding_cmp(
		(ccs_binding_t)configuration_ref, (ccs_binding_t)configuration,
		&cmp);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!cmp);

	free(buff);
	err = ccs_release_object(configuration_space);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(configuration_ref);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(configuration);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(map);
	assert(err == CCS_RESULT_SUCCESS);
	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
}

void
test_deserialize(void)
{
	ccs_parameter_t           parameters[3];
	ccs_expression_t          conditions[3] = {NULL, NULL, NULL};
	ccs_configuration_space_t space, space_ref;
	ccs_expression_t          expression, expressions[3];
	char                     *buff;
	size_t                    buff_size;
	size_t                    count;
	ccs_map_t                 map;
	ccs_datum_t               d;
	ccs_result_t              err;

	parameters[0] = create_numerical("param1", -5.0, 5.0);
	parameters[1] = create_numerical("param2", -5.0, 5.0);
	parameters[2] = create_numerical("param3", -5.0, 5.0);

	err           = ccs_create_binary_expression(
                CCS_EXPRESSION_TYPE_LESS, ccs_object(parameters[0]),
                ccs_float(0.0), &expression);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_binary_expression(
		CCS_EXPRESSION_TYPE_LESS, ccs_object(parameters[1]),
		ccs_float(0.0), &conditions[2]);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_binary_expression(
		CCS_EXPRESSION_TYPE_LESS, ccs_object(parameters[2]),
		ccs_float(0.0), &conditions[0]);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_configuration_space(
		"my_config_space", 3, parameters, conditions, 1, &expression,
		NULL, NULL, &space);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(conditions[0]);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(conditions[2]);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}

	err = ccs_object_serialize(
		space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	space_ref = space;
	err       = ccs_release_object(space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_DESERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_map(&map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_object_deserialize(
		(ccs_object_t *)&space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_DESERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_MAP_HANDLES, CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_map_get(map, ccs_object(space_ref), &d);
	assert(err == CCS_RESULT_SUCCESS);
	assert(d.type == CCS_DATA_TYPE_OBJECT);
	assert(d.value.o == space);

	err = ccs_context_get_parameters((ccs_context_t)space, 0, NULL, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 3);

	err = ccs_configuration_space_get_conditions(
		space, 3, expressions, NULL);
	assert(err == CCS_RESULT_SUCCESS);
	assert(expressions[0]);
	assert(!expressions[1]);
	assert(expressions[2]);
	assert(expressions[0] != expressions[2]);

	err = ccs_configuration_space_get_forbidden_clauses(
		space, 3, expressions, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 1);
	assert(expressions[0]);
	assert(!expressions[1]);
	assert(!expressions[2]);

	err = ccs_release_object(map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(space);
	assert(err == CCS_RESULT_SUCCESS);
	free(buff);
}

void
test_deserialize_errors(void)
{
	ccs_rng_t    rng;
	ccs_object_t obj;
	ccs_result_t err;
	char        *buff;
	char        *corrupt;
	size_t       buff_size;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_serialize(
		rng, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE,
		&buff_size, CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		rng, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	/* Truncated buffer — too short for magic tag */
	err = ccs_object_deserialize(
		&obj, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_DESERIALIZE_OPERATION_MEMORY, 2, buff,
		CCS_DESERIALIZE_OPTION_END);
	assert(err != CCS_RESULT_SUCCESS);
	ccs_clear_thread_error();

	/* Wrong magic tag */
	corrupt = (char *)malloc(buff_size);
	assert(corrupt);
	memcpy(corrupt, buff, buff_size);
	corrupt[0] = 'X';
	corrupt[1] = 'Y';
	corrupt[2] = 'Z';
	err        = ccs_object_deserialize(
                &obj, CCS_SERIALIZE_FORMAT_BINARY,
                CCS_DESERIALIZE_OPERATION_MEMORY, buff_size, corrupt,
                CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_clear_thread_error();

	/* Version too high — patch the version byte after magic(4) + size(8) */
	memcpy(corrupt, buff, buff_size);
	corrupt[12] = (char)0xFF;
	err         = ccs_object_deserialize(
                &obj, CCS_SERIALIZE_FORMAT_BINARY,
                CCS_DESERIALIZE_OPERATION_MEMORY, buff_size, corrupt,
                CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_clear_thread_error();

	free(corrupt);
	free(buff);
	ccs_release_object(rng);
}

void
test_configuration_with_features(void)
{
	ccs_result_t              err;
	ccs_parameter_t           parameter;
	ccs_configuration_space_t cspace;
	ccs_configuration_t       config1, config2;
	ccs_feature_space_t       fspace;
	ccs_features_t            features_on, features_off;
	ccs_features_t            features_ret;
	ccs_hash_t                hash1, hash2;
	int                       cmp;

	parameter = create_numerical("x", -5.0, 5.0);

	fspace    = create_knobs(&features_on, &features_off);

	err       = ccs_create_configuration_space(
                "space_with_features", 1, &parameter, NULL, 0, NULL, fspace,
                NULL, &cspace);
	assert(err == CCS_RESULT_SUCCESS);

	/* Sample a configuration with features */
	err = ccs_configuration_space_sample(
		cspace, NULL, features_on, NULL, &config1);
	assert(err == CCS_RESULT_SUCCESS);

	/* Verify get_features */
	err = ccs_configuration_get_features(config1, &features_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(features_ret == features_on);

	/* Sample with different features */
	err = ccs_configuration_space_sample(
		cspace, NULL, features_off, NULL, &config2);
	assert(err == CCS_RESULT_SUCCESS);

	/* Hash should work with features */
	err = ccs_binding_hash((ccs_binding_t)config1, &hash1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_binding_hash((ccs_binding_t)config2, &hash2);
	assert(err == CCS_RESULT_SUCCESS);

	/* Compare configurations with different features */
	err = ccs_binding_cmp(
		(ccs_binding_t)config1, (ccs_binding_t)config2, &cmp);
	assert(err == CCS_RESULT_SUCCESS);
	assert(cmp != 0);

	/* Compare configuration with itself */
	err = ccs_binding_cmp(
		(ccs_binding_t)config1, (ccs_binding_t)config1, &cmp);
	assert(err == CCS_RESULT_SUCCESS);
	assert(cmp == 0);

	ccs_release_object(config2);
	ccs_release_object(config1);
	ccs_release_object(cspace);
	ccs_release_object(features_off);
	ccs_release_object(features_on);
	ccs_release_object(fspace);
	ccs_release_object(parameter);
}

int
main(void)
{
	ccs_init();
	test_create();
	test_empty();
	test_sample();
	test_deserialize();
	test_configuration_deserialize();
	test_deserialize_errors();
	test_configuration_with_features();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
