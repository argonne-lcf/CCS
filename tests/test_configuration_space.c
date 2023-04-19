#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

double d = -2.0;

ccs_parameter_t
create_dummy_parameter(const char *name)
{
	ccs_parameter_t parameter;
	ccs_error_t     err;
	err = ccs_create_numerical_parameter(
		name, CCS_NUM_FLOAT, CCSF(-5.0), CCSF(5.0), CCSF(0.0), CCSF(d),
		&parameter);
	d += 1.0;
	if (d >= 5.0)
		d = -5.0;
	assert(err == CCS_SUCCESS);
	return parameter;
}

void
test_create()
{
	ccs_configuration_space_t configuration_space;
	ccs_error_t               err;
	ccs_object_type_t         type;
	const char               *name;
	size_t                    sz;

	err = ccs_create_configuration_space(
		"my_config_space", &configuration_space);
	assert(err == CCS_SUCCESS);

	err = ccs_object_get_type(configuration_space, &type);
	assert(err == CCS_SUCCESS);
	assert(type == CCS_OBJECT_TYPE_CONFIGURATION_SPACE);

	err = ccs_configuration_space_get_name(configuration_space, &name);
	assert(err == CCS_SUCCESS);
	assert(strcmp(name, "my_config_space") == 0);

	err = ccs_configuration_space_get_num_parameters(
		configuration_space, &sz);
	assert(err == CCS_SUCCESS);
	assert(sz == 0);

	err = ccs_release_object(configuration_space);
	assert(err == CCS_SUCCESS);
}

void
check_configuration(
	ccs_configuration_space_t configuration_space,
	size_t                    sz,
	ccs_parameter_t          *parameters)
{
	ccs_parameter_t     parameter;
	ccs_configuration_t configuration;
	ccs_error_t         err;
	size_t              sz_ret;
	size_t              index;
	ccs_parameter_t    *parameters_ret =
		(ccs_parameter_t *)malloc(sizeof(ccs_parameter_t) * (sz + 1));
	const char *name;
	ccs_bool_t  check;

	err = ccs_configuration_space_get_num_parameters(
		configuration_space, &sz_ret);
	assert(err == CCS_SUCCESS);
	assert(sz_ret == sz);

	for (size_t i = 0; i < sz; i++) {
		err = ccs_configuration_space_get_parameter(
			configuration_space, i, &parameter);
		assert(err == CCS_SUCCESS);
		assert(parameter == parameters[i]);
		err = ccs_configuration_space_get_parameter_index(
			configuration_space, parameter, &index);
		assert(err == CCS_SUCCESS);
		assert(index == i);
	}
	err = ccs_configuration_space_get_parameters(
		configuration_space, sz + 1, parameters_ret, &sz_ret);
	assert(err == CCS_SUCCESS);
	assert(sz_ret == sz);
	for (size_t i = 0; i < sz; i++)
		assert(parameters_ret[i] == parameters[i]);
	assert(parameters_ret[sz] == NULL);

	for (size_t i = 0; i < sz; i++) {
		err = ccs_parameter_get_name(parameters[i], &name);
		assert(err == CCS_SUCCESS);
		err = ccs_configuration_space_get_parameter_by_name(
			configuration_space, name, &parameter);
		assert(err == CCS_SUCCESS);
		assert(parameter == parameters[i]);
	}

	err = ccs_configuration_space_get_default_configuration(
		configuration_space, &configuration);
	assert(err == CCS_SUCCESS);

	err = ccs_configuration_check(configuration, &check);
	assert(err == CCS_SUCCESS);
	assert(check);
	for (size_t i = 0; i < sz; i++) {
		ccs_datum_t datum;
		ccs_datum_t hdatum;
		err = ccs_configuration_get_value(configuration, i, &datum);
		assert(err == CCS_SUCCESS);
		err = ccs_parameter_get_default_value(parameters[i], &hdatum);
		assert(err == CCS_SUCCESS);
		assert(datum.type == hdatum.type);
		assert(datum.value.f == hdatum.value.f);
	}
	ccs_release_object(configuration);
	free(parameters_ret);
}

void
test_add()
{
	ccs_parameter_t           parameters[3];
	ccs_configuration_space_t configuration_space;
	ccs_error_t               err;

	err = ccs_create_configuration_space(
		"my_config_space", &configuration_space);
	assert(err == CCS_SUCCESS);

	parameters[0] = create_dummy_parameter("param1");
	parameters[1] = create_dummy_parameter("param2");
	parameters[2] = create_dummy_parameter("param3");

	err           = ccs_configuration_space_add_parameter(
                configuration_space, parameters[0], NULL);
	assert(err == CCS_SUCCESS);

	err = ccs_configuration_space_add_parameter(
		configuration_space, parameters[0], NULL);
	assert(err == CCS_INVALID_PARAMETER);

	err = ccs_configuration_space_add_parameter(
		configuration_space, parameters[1], NULL);
	assert(err == CCS_SUCCESS);
	err = ccs_configuration_space_add_parameter(
		configuration_space, parameters[2], NULL);
	assert(err == CCS_SUCCESS);

	check_configuration(configuration_space, 3, parameters);

	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_SUCCESS);
	}
	err = ccs_release_object(configuration_space);
	assert(err == CCS_SUCCESS);
}

void
test_add_list()
{
	ccs_parameter_t           parameters[3];
	ccs_configuration_space_t configuration_space;
	ccs_error_t               err;

	err = ccs_create_configuration_space(
		"my_config_space", &configuration_space);
	assert(err == CCS_SUCCESS);

	parameters[0] = create_dummy_parameter("param1");
	parameters[1] = create_dummy_parameter("param2");
	parameters[2] = create_dummy_parameter("param3");

	err           = ccs_configuration_space_add_parameters(
                configuration_space, 3, parameters, NULL);
	assert(err == CCS_SUCCESS);

	check_configuration(configuration_space, 3, parameters);

	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_SUCCESS);
	}
	err = ccs_release_object(configuration_space);
	assert(err == CCS_SUCCESS);
}

void
test_sample()
{
	ccs_parameter_t           parameters[4];
	ccs_configuration_t       configuration;
	ccs_configuration_t       configurations[100];
	ccs_configuration_space_t configuration_space;
	ccs_error_t               err;
	ccs_bool_t                check;

	err = ccs_create_configuration_space(
		"my_config_space", &configuration_space);
	assert(err == CCS_SUCCESS);

	parameters[0] = create_dummy_parameter("param1");
	parameters[1] = create_dummy_parameter("param2");
	parameters[2] = create_dummy_parameter("param3");
	err           = ccs_create_numerical_parameter(
                "param4", CCS_NUM_INTEGER, CCSI(-5), CCSI(5), CCSI(0), CCSI(0),
                parameters + 3);
	assert(err == CCS_SUCCESS);

	err = ccs_configuration_space_add_parameters(
		configuration_space, 4, parameters, NULL);
	assert(err == CCS_SUCCESS);

	err = ccs_configuration_space_sample(
		configuration_space, &configuration);
	assert(err == CCS_SUCCESS);

	err = ccs_configuration_check(configuration, &check);
	assert(err == CCS_SUCCESS);
	assert(check);

	err = ccs_configuration_space_samples(
		configuration_space, 100, configurations);
	assert(err == CCS_SUCCESS);

	for (size_t i = 0; i < 100; i++) {
		err = ccs_configuration_check(configurations[i], &check);
		assert(err == CCS_SUCCESS);
		assert(check);
		err = ccs_release_object(configurations[i]);
		assert(err == CCS_SUCCESS);
	}

	err = ccs_release_object(configuration);
	assert(err == CCS_SUCCESS);
	for (size_t i = 0; i < 4; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_SUCCESS);
	}
	err = ccs_release_object(configuration_space);
	assert(err == CCS_SUCCESS);
}

void
test_set_distribution()
{
	ccs_parameter_t           parameters[3];
	ccs_distribution_t        distribs[2];
	ccs_distribution_t        distrib;
	ccs_distribution_t        distrib_ret;
	size_t                    hindexes[2];
	size_t                    dindex_ret;
	ccs_configuration_t       configurations[100];
	ccs_configuration_space_t configuration_space;
	ccs_error_t               err;
	ccs_bool_t                check;

	err = ccs_create_configuration_space(
		"my_config_space", &configuration_space);
	assert(err == CCS_SUCCESS);

	parameters[0] = create_dummy_parameter("param1");
	parameters[1] = create_dummy_parameter("param2");
	parameters[2] = create_dummy_parameter("param3");

	err           = ccs_configuration_space_add_parameters(
                configuration_space, 3, parameters, NULL);
	assert(err == CCS_SUCCESS);

	check_configuration(configuration_space, 3, parameters);

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT, CCSF(-4.0), CCSF(4.0), CCS_SCALE_TYPE_LINEAR,
		CCSF(0.0), distribs);
	assert(err == CCS_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT, CCSF(-3.0), CCSF(3.0), CCS_SCALE_TYPE_LINEAR,
		CCSF(0.0), distribs + 1);
	assert(err == CCS_SUCCESS);

	err = ccs_create_multivariate_distribution(2, distribs, &distrib);
	assert(err == CCS_SUCCESS);

	hindexes[0] = 0;
	hindexes[1] = 1;
	err         = ccs_configuration_space_set_distribution(
                configuration_space, distrib, hindexes);
	assert(err == CCS_SUCCESS);

	err = ccs_configuration_space_get_parameter_distribution(
		configuration_space, 0, &distrib_ret, &dindex_ret);
	assert(err == CCS_SUCCESS);
	assert(distrib_ret == distrib);
	assert(dindex_ret == 0);

	err = ccs_configuration_space_get_parameter_distribution(
		configuration_space, 1, &distrib_ret, &dindex_ret);
	assert(err == CCS_SUCCESS);
	assert(distrib_ret == distrib);
	assert(dindex_ret == 1);

	err = ccs_configuration_space_samples(
		configuration_space, 100, configurations);
	assert(err == CCS_SUCCESS);

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t values[3];
		err = ccs_configuration_check(configurations[i], &check);
		assert(err == CCS_SUCCESS);
		assert(check);
		err = ccs_configuration_get_values(
			configurations[i], 3, values, NULL);
		assert(err == CCS_SUCCESS);
		assert(values[0].value.f >= -4.0);
		assert(values[0].value.f < 4.0);
		assert(values[1].value.f >= -3.0);
		assert(values[1].value.f < 3.0);
		err = ccs_release_object(configurations[i]);
		assert(err == CCS_SUCCESS);
	}

	hindexes[0] = 2;
	hindexes[1] = 0;
	err         = ccs_configuration_space_set_distribution(
                configuration_space, distrib, hindexes);
	assert(err == CCS_SUCCESS);

	err = ccs_configuration_space_get_parameter_distribution(
		configuration_space, 0, &distrib_ret, &dindex_ret);
	assert(err == CCS_SUCCESS);
	assert(distrib_ret == distrib);
	assert(dindex_ret == 1);

	err = ccs_configuration_space_get_parameter_distribution(
		configuration_space, 2, &distrib_ret, &dindex_ret);
	assert(err == CCS_SUCCESS);
	assert(distrib_ret == distrib);
	assert(dindex_ret == 0);

	err = ccs_configuration_space_samples(
		configuration_space, 100, configurations);
	assert(err == CCS_SUCCESS);

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t values[3];
		err = ccs_configuration_check(configurations[i], &check);
		assert(err == CCS_SUCCESS);
		assert(check);
		err = ccs_configuration_get_values(
			configurations[i], 3, values, NULL);
		assert(err == CCS_SUCCESS);
		assert(values[2].value.f >= -4.0);
		assert(values[2].value.f < 4.0);
		assert(values[0].value.f >= -3.0);
		assert(values[0].value.f < 3.0);
		err = ccs_release_object(configurations[i]);
		assert(err == CCS_SUCCESS);
	}

	for (size_t i = 0; i < 2; i++) {
		err = ccs_release_object(distribs[i]);
		assert(err == CCS_SUCCESS);
	}
	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_SUCCESS);
	}
	err = ccs_release_object(distrib);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(configuration_space);
	assert(err == CCS_SUCCESS);
}

ccs_parameter_t
create_numerical(const char *name)
{
	ccs_parameter_t parameter;
	ccs_error_t     err;
	err = ccs_create_numerical_parameter(
		name, CCS_NUM_FLOAT, CCSF(-5.0), CCSF(5.0), CCSF(0.0),
		CCSF(0.0), &parameter);
	assert(err == CCS_SUCCESS);
	return parameter;
}

void
test_configuration_deserialize()
{
	ccs_parameter_t           parameters[3];
	ccs_configuration_space_t configuration_space;
	ccs_configuration_t       configuration, configuration_ref;
	char                     *buff;
	size_t                    buff_size;
	ccs_map_t                 map;
	ccs_datum_t               d;
	ccs_error_t               err;
	int                       cmp;

	err = ccs_create_configuration_space(
		"my_config_space", &configuration_space);
	assert(err == CCS_SUCCESS);

	parameters[0] = create_numerical("param1");
	parameters[1] = create_numerical("param2");
	parameters[2] = create_numerical("param3");
	err           = ccs_configuration_space_add_parameters(
                configuration_space, 3, parameters, NULL);
	assert(err == CCS_SUCCESS);
	err = ccs_configuration_space_sample(
		configuration_space, &configuration_ref);
	assert(err == CCS_SUCCESS);

	err = ccs_create_map(&map);
	assert(err == CCS_SUCCESS);
	err = ccs_object_serialize(
		configuration_ref, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);
	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		configuration_ref, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&configuration, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_INVALID_HANDLE);

	d = ccs_object(configuration_space);
	d.flags |= CCS_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(configuration_space));
	assert(err == CCS_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&configuration, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);

	err = ccs_configuration_cmp(configuration_ref, configuration, &cmp);
	assert(err == CCS_SUCCESS);
	assert(!cmp);

	free(buff);
	err = ccs_release_object(configuration_space);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(configuration_ref);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(configuration);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(map);
	assert(err == CCS_SUCCESS);
	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_SUCCESS);
	}
}

void
test_deserialize()
{
	ccs_parameter_t           parameters[3];
	ccs_configuration_space_t space, space_ref;
	ccs_expression_t          expression, expressions[3];
	ccs_distribution_t        distribs[2];
	ccs_distribution_t        distrib;
	size_t                    hindexes[2];
	ccs_distribution_t        distrib_ret, distrib_ref;
	size_t                    dindex_ret;
	char                     *buff;
	size_t                    buff_size;
	size_t                    count;
	ccs_map_t                 map;
	ccs_datum_t               d;
	ccs_error_t               err;

	err = ccs_create_configuration_space("my_config_space", &space);
	assert(err == CCS_SUCCESS);

	parameters[0] = create_numerical("param1");
	parameters[1] = create_numerical("param2");
	parameters[2] = create_numerical("param3");

	err           = ccs_configuration_space_add_parameters(
                space, 3, parameters, NULL);
	assert(err == CCS_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT, CCSF(-4.0), CCSF(4.0), CCS_SCALE_TYPE_LINEAR,
		CCSF(0.0), distribs);
	assert(err == CCS_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT, CCSF(-3.0), CCSF(3.0), CCS_SCALE_TYPE_LINEAR,
		CCSF(0.0), distribs + 1);
	assert(err == CCS_SUCCESS);

	err = ccs_create_multivariate_distribution(2, distribs, &distrib);
	assert(err == CCS_SUCCESS);

	hindexes[0] = 2;
	hindexes[1] = 0;
	err         = ccs_configuration_space_set_distribution(
                space, distrib, hindexes);
	assert(err == CCS_SUCCESS);

	err = ccs_create_binary_expression(
		CCS_EXPRESSION_TYPE_LESS, ccs_object(parameters[1]),
		ccs_float(0.0), &expression);
	assert(err == CCS_SUCCESS);
	err = ccs_configuration_space_set_condition(space, 2, expression);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_SUCCESS);

	err = ccs_create_binary_expression(
		CCS_EXPRESSION_TYPE_LESS, ccs_object(parameters[2]),
		ccs_float(0.0), &expression);
	assert(err == CCS_SUCCESS);
	err = ccs_configuration_space_set_condition(space, 0, expression);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_SUCCESS);

	err = ccs_create_binary_expression(
		CCS_EXPRESSION_TYPE_LESS, ccs_object(parameters[0]),
		ccs_float(0.0), &expression);
	assert(err == CCS_SUCCESS);
	err = ccs_configuration_space_add_forbidden_clause(space, expression);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_SUCCESS);

	for (size_t i = 0; i < 2; i++) {
		err = ccs_release_object(distribs[i]);
		assert(err == CCS_SUCCESS);
	}
	err = ccs_release_object(distrib);
	assert(err == CCS_SUCCESS);
	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_SUCCESS);
	}

	err = ccs_object_serialize(
		space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);

	space_ref = space;
	err       = ccs_release_object(space);
	assert(err == CCS_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);

	err = ccs_release_object(space);
	assert(err == CCS_SUCCESS);

	err = ccs_create_map(&map);
	assert(err == CCS_SUCCESS);
	err = ccs_object_deserialize(
		(ccs_object_t *)&space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);

	err = ccs_map_get(map, ccs_object(space_ref), &d);
	assert(err == CCS_SUCCESS);
	assert(d.type == CCS_OBJECT);
	assert(d.value.o == space);

	err = ccs_configuration_space_get_parameters(space, 0, NULL, &count);
	assert(err == CCS_SUCCESS);
	assert(count == 3);

	err = ccs_configuration_space_get_parameter_distribution(
		space, 0, &distrib_ret, &dindex_ret);
	assert(err == CCS_SUCCESS);
	assert(distrib_ret);
	assert(dindex_ret == 1);
	distrib_ref = distrib_ret;

	err         = ccs_configuration_space_get_parameter_distribution(
                space, 2, &distrib_ret, &dindex_ret);
	assert(err == CCS_SUCCESS);
	assert(distrib_ret == distrib_ref);
	assert(dindex_ret == 0);

	err = ccs_configuration_space_get_conditions(
		space, 3, expressions, NULL);
	assert(err == CCS_SUCCESS);
	assert(expressions[0]);
	assert(!expressions[1]);
	assert(expressions[2]);
	assert(expressions[0] != expressions[2]);

	err = ccs_configuration_space_get_forbidden_clauses(
		space, 3, expressions, &count);
	assert(err == CCS_SUCCESS);
	assert(count == 1);
	assert(expressions[0]);
	assert(!expressions[1]);
	assert(!expressions[2]);

	err = ccs_release_object(map);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(space);
	assert(err == CCS_SUCCESS);
	free(buff);
}

int
main()
{
	ccs_init();
	test_create();
	test_add();
	test_add_list();
	test_sample();
	test_set_distribution();
	test_deserialize();
	test_configuration_deserialize();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
