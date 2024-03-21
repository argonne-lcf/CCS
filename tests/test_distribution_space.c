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
	ccs_bool_t  check;

	err = ccs_configuration_space_get_num_parameters(
		configuration_space, &sz_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(sz_ret == sz);

	for (size_t i = 0; i < sz; i++) {
		err = ccs_configuration_space_get_parameter(
			configuration_space, i, &parameter);
		assert(err == CCS_RESULT_SUCCESS);
		assert(parameter == parameters[i]);
		err = ccs_configuration_space_get_parameter_index(
			configuration_space, parameter, NULL, &index);
		assert(err == CCS_RESULT_SUCCESS);
		assert(index == i);
	}
	err = ccs_configuration_space_get_parameters(
		configuration_space, sz + 1, parameters_ret, &sz_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(sz_ret == sz);
	for (size_t i = 0; i < sz; i++)
		assert(parameters_ret[i] == parameters[i]);
	assert(parameters_ret[sz] == NULL);

	for (size_t i = 0; i < sz; i++) {
		err = ccs_parameter_get_name(parameters[i], &name);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_configuration_space_get_parameter_by_name(
			configuration_space, name, &parameter);
		assert(err == CCS_RESULT_SUCCESS);
		assert(parameter == parameters[i]);
	}

	err = ccs_configuration_space_get_default_configuration(
		configuration_space, &configuration);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_configuration_check(configuration, &check);
	assert(err == CCS_RESULT_SUCCESS);
	assert(check);
	for (size_t i = 0; i < sz; i++) {
		ccs_datum_t datum;
		ccs_datum_t hdatum;
		err = ccs_configuration_get_value(configuration, i, &datum);
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
test_create(void)
{
	ccs_parameter_t           parameters[3];
	ccs_configuration_space_t configuration_space, cs_ret;
	ccs_distribution_space_t  distribution_space;
	ccs_result_t              err;
	ccs_object_type_t         type;

	parameters[0] = create_dummy_parameter("param1");
	parameters[1] = create_dummy_parameter("param2");
	parameters[2] = create_dummy_parameter("param3");

	err           = ccs_create_configuration_space(
                "my_config_space", 3, parameters, NULL, 0, NULL,
                &configuration_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_distribution_space(
		configuration_space, &distribution_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_get_type(distribution_space, &type);
	assert(err == CCS_RESULT_SUCCESS);
	assert(type == CCS_OBJECT_TYPE_DISTRIBUTION_SPACE);

	err = ccs_distribution_space_get_configuration_space(
		distribution_space, &cs_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(configuration_space == cs_ret);

	err = ccs_release_object(configuration_space);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(distribution_space);
	assert(err == CCS_RESULT_SUCCESS);
	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
}

void
test_set_distribution(void)
{
	ccs_parameter_t           parameters[3];
	ccs_distribution_space_t  distribution_space;
	ccs_distribution_t        distribs[2];
	ccs_distribution_t        distrib;
	ccs_distribution_t        distrib_ret;
	size_t                    hindexes[2];
	size_t                    dindex_ret;
	ccs_configuration_t       configurations[100];
	ccs_configuration_space_t configuration_space;
	ccs_result_t              err;
	ccs_bool_t                check;

	parameters[0] = create_dummy_parameter("param1");
	parameters[1] = create_dummy_parameter("param2");
	parameters[2] = create_dummy_parameter("param3");

	err           = ccs_create_configuration_space(
                "my_config_space", 3, parameters, NULL, 0, NULL,
                &configuration_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_distribution_space(
		configuration_space, &distribution_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_FLOAT, CCSF(-4.0), CCSF(4.0),
		CCS_SCALE_TYPE_LINEAR, CCSF(0.0), distribs);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_FLOAT, CCSF(-3.0), CCSF(3.0),
		CCS_SCALE_TYPE_LINEAR, CCSF(0.0), distribs + 1);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_multivariate_distribution(2, distribs, &distrib);
	assert(err == CCS_RESULT_SUCCESS);

	hindexes[0] = 0;
	hindexes[1] = 1;
	err         = ccs_distribution_space_set_distribution(
                distribution_space, distrib, hindexes);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_space_get_parameter_distribution(
		distribution_space, 0, &distrib_ret, &dindex_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(distrib_ret == distrib);
	assert(dindex_ret == 0);

	err = ccs_distribution_space_get_parameter_distribution(
		distribution_space, 1, &distrib_ret, &dindex_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(distrib_ret == distrib);
	assert(dindex_ret == 1);

	err = ccs_configuration_space_samples(
		configuration_space, distribution_space, NULL, 100,
		configurations);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t values[3];
		err = ccs_configuration_check(configurations[i], &check);
		assert(err == CCS_RESULT_SUCCESS);
		assert(check);
		err = ccs_configuration_get_values(
			configurations[i], 3, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		assert(values[0].value.f >= -4.0);
		assert(values[0].value.f < 4.0);
		assert(values[1].value.f >= -3.0);
		assert(values[1].value.f < 3.0);
		err = ccs_release_object(configurations[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}

	hindexes[0] = 2;
	hindexes[1] = 0;
	err         = ccs_distribution_space_set_distribution(
                distribution_space, distrib, hindexes);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_space_get_parameter_distribution(
		distribution_space, 0, &distrib_ret, &dindex_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(distrib_ret == distrib);
	assert(dindex_ret == 1);

	err = ccs_distribution_space_get_parameter_distribution(
		distribution_space, 2, &distrib_ret, &dindex_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(distrib_ret == distrib);
	assert(dindex_ret == 0);

	err = ccs_configuration_space_samples(
		configuration_space, distribution_space, NULL, 100,
		configurations);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t values[3];
		err = ccs_configuration_check(configurations[i], &check);
		assert(err == CCS_RESULT_SUCCESS);
		assert(check);
		err = ccs_configuration_get_values(
			configurations[i], 3, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		assert(values[2].value.f >= -4.0);
		assert(values[2].value.f < 4.0);
		assert(values[0].value.f >= -3.0);
		assert(values[0].value.f < 3.0);
		err = ccs_release_object(configurations[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}

	for (size_t i = 0; i < 2; i++) {
		err = ccs_release_object(distribs[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(configuration_space);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(distribution_space);
	assert(err == CCS_RESULT_SUCCESS);
}

ccs_parameter_t
create_numerical(const char *name)
{
	ccs_parameter_t parameter;
	ccs_result_t    err;
	err = ccs_create_numerical_parameter(
		name, CCS_NUMERIC_TYPE_FLOAT, CCSF(-5.0), CCSF(5.0), CCSF(0.0),
		CCSF(0.0), &parameter);
	assert(err == CCS_RESULT_SUCCESS);
	return parameter;
}

void
test_deserialize(void)
{
	ccs_parameter_t           parameters[3];
	ccs_configuration_space_t space;
	ccs_distribution_space_t  distrib_space;
	ccs_distribution_t        distribs[2];
	ccs_distribution_t        distrib;
	size_t                    hindexes[2];
	ccs_distribution_t        distrib_ret, distrib_ref;
	size_t                    dindex_ret;
	char                     *buff;
	size_t                    buff_size;
	ccs_map_t                 map;
	ccs_datum_t               d;
	ccs_result_t              err;

	parameters[0] = create_numerical("param1");
	parameters[1] = create_numerical("param2");
	parameters[2] = create_numerical("param3");

	err           = ccs_create_configuration_space(
                "my_config_space", 3, parameters, NULL, 0, NULL, &space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_distribution_space(space, &distrib_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_FLOAT, CCSF(-4.0), CCSF(4.0),
		CCS_SCALE_TYPE_LINEAR, CCSF(0.0), distribs);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_FLOAT, CCSF(-3.0), CCSF(3.0),
		CCS_SCALE_TYPE_LINEAR, CCSF(0.0), distribs + 1);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_multivariate_distribution(2, distribs, &distrib);
	assert(err == CCS_RESULT_SUCCESS);

	hindexes[0] = 2;
	hindexes[1] = 0;
	err         = ccs_distribution_space_set_distribution(
                distrib_space, distrib, hindexes);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t i = 0; i < 2; i++) {
		err = ccs_release_object(distribs[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);
	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}

	err = ccs_object_serialize(
		distrib_space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		distrib_space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_map(&map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(distrib_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&distrib_space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_HANDLE);

	d = ccs_object(space);
	d.flags |= CCS_DATUM_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(space));
	assert(err == CCS_RESULT_SUCCESS);

	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_object_deserialize(
		(ccs_object_t *)&distrib_space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_space_get_parameter_distribution(
		distrib_space, 0, &distrib_ret, &dindex_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(distrib_ret);
	assert(dindex_ret == 1);
	distrib_ref = distrib_ret;

	err         = ccs_distribution_space_get_parameter_distribution(
                distrib_space, 2, &distrib_ret, &dindex_ret);
	assert(err == CCS_RESULT_SUCCESS);
	assert(distrib_ret == distrib_ref);
	assert(dindex_ret == 0);

	err = ccs_release_object(map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(space);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(distrib_space);
	assert(err == CCS_RESULT_SUCCESS);
	free(buff);
}

int
main(void)
{
	ccs_init();
	test_create();
	test_set_distribution();
	test_deserialize();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
