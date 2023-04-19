#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>
#include <math.h>

ccs_parameter_t
create_numerical(const char *name)
{
	ccs_parameter_t parameter;
	ccs_error_t     err;
	err = ccs_create_numerical_parameter(
		name, CCS_NUM_FLOAT, CCSF(-1.0), CCSF(1.0), CCSF(0.0), CCSF(0),
		&parameter);
	assert(err == CCS_SUCCESS);
	return parameter;
}

void
test_simple()
{
	ccs_parameter_t           parameter1, parameter2;
	ccs_configuration_space_t space;
	ccs_expression_t          expression;
	ccs_configuration_t       configuration;
	ccs_datum_t               values[2];
	ccs_configuration_t       configurations[100];
	ccs_error_t               err;

	parameter1 = create_numerical("param1");
	parameter2 = create_numerical("param2");
	err        = ccs_create_configuration_space("space", &space);
	assert(err == CCS_SUCCESS);
	err = ccs_configuration_space_add_parameter(space, parameter1, NULL);
	assert(err == CCS_SUCCESS);
	err = ccs_configuration_space_add_parameter(space, parameter2, NULL);
	assert(err == CCS_SUCCESS);

	err = ccs_create_binary_expression(
		CCS_EXPRESSION_TYPE_LESS, ccs_object(parameter1), ccs_float(0.0), &expression);
	assert(err == CCS_SUCCESS);

	err = ccs_configuration_space_add_forbidden_clause(space, expression);
	assert(err == CCS_SUCCESS);

	for (int i = 0; i < 100; i++) {
		ccs_float_t f;
		ccs_bool_t  check;
		err = ccs_configuration_space_sample(space, &configuration);
		assert(err == CCS_SUCCESS);
		err = ccs_configuration_get_values(
			configuration, 2, values, NULL);
		assert(err == CCS_SUCCESS);
		assert(values[0].type == CCS_FLOAT);
		f = values[0].value.f;
		assert(f >= 0.0 && f < 1.0);
		assert(values[1].type == CCS_FLOAT);
		f = values[0].value.f;
		assert(f >= -1.0 && f < 1.0);
		err = ccs_configuration_space_check_configuration(
			space, configuration, &check);
		assert(err == CCS_SUCCESS);
		assert(check);
		err = ccs_release_object(configuration);
		assert(err == CCS_SUCCESS);
	}
	err = ccs_configuration_space_samples(space, 100, configurations);
	assert(err == CCS_SUCCESS);

	for (int i = 0; i < 100; i++) {
		ccs_float_t f;
		ccs_bool_t  check;
		err = ccs_configuration_get_values(
			configurations[i], 2, values, NULL);
		assert(err == CCS_SUCCESS);
		assert(values[0].type == CCS_FLOAT);
		f = values[0].value.f;
		assert(f >= 0.0 && f < 1.0);
		assert(values[1].type == CCS_FLOAT);
		f = values[0].value.f;
		assert(f >= -1.0 && f < 1.0);
		err = ccs_configuration_space_check_configuration(
			space, configurations[i], &check);
		assert(err == CCS_SUCCESS);
		assert(check);
		err = ccs_release_object(configurations[i]);
		assert(err == CCS_SUCCESS);
	}

	err = ccs_release_object(expression);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(parameter1);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(parameter2);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(space);
	assert(err == CCS_SUCCESS);
}

void
test_combined()
{
	ccs_parameter_t           parameters[3];
	ccs_configuration_space_t space;
	ccs_expression_t          expression;
	ccs_configuration_t       configuration;
	ccs_datum_t               values[3];
	ccs_configuration_t       configurations[100];
	ccs_error_t               err;

	parameters[0] = create_numerical("param1");
	parameters[1] = create_numerical("param2");
	parameters[2] = create_numerical("param3");

	err           = ccs_create_configuration_space("space", &space);
	assert(err == CCS_SUCCESS);

	err = ccs_configuration_space_add_parameters(
		space, 3, parameters, NULL);
	assert(err == CCS_SUCCESS);

	err = ccs_create_binary_expression(
		CCS_EXPRESSION_TYPE_LESS, ccs_object(parameters[1]), ccs_float(0.0),
		&expression);
	assert(err == CCS_SUCCESS);
	err = ccs_configuration_space_set_condition(space, 2, expression);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_SUCCESS);

	err = ccs_create_binary_expression(
		CCS_EXPRESSION_TYPE_LESS, ccs_object(parameters[2]), ccs_float(0.0),
		&expression);
	assert(err == CCS_SUCCESS);
	err = ccs_configuration_space_set_condition(space, 0, expression);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_SUCCESS);

	err = ccs_create_binary_expression(
		CCS_EXPRESSION_TYPE_LESS, ccs_object(parameters[0]), ccs_float(0.0),
		&expression);
	assert(err == CCS_SUCCESS);
	err = ccs_configuration_space_add_forbidden_clause(space, expression);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_SUCCESS);

	for (int i = 0; i < 100; i++) {
		ccs_float_t f;
		ccs_bool_t  check;
		err = ccs_configuration_space_sample(space, &configuration);
		assert(err == CCS_SUCCESS);
		err = ccs_configuration_get_values(
			configuration, 3, values, NULL);
		assert(err == CCS_SUCCESS);
		assert(values[1].type == CCS_FLOAT);
		f = values[1].value.f;
		assert(f >= -1.0 && f < 1.0);
		if (f < 0.0) {
			assert(values[2].type == CCS_FLOAT);
			f = values[2].value.f;
			assert(f >= -1.0 && f < 1.0);
			if (f < 0.0) {
				assert(values[0].type == CCS_FLOAT);
				f = values[0].value.f;
				assert(f >= 0.0 && f < 1.0);
			} else
				assert(values[0].type == CCS_INACTIVE);
		} else {
			assert(values[2].type == CCS_INACTIVE);
			assert(values[0].type == CCS_INACTIVE);
		}
		err = ccs_configuration_space_check_configuration(
			space, configuration, &check);
		assert(err == CCS_SUCCESS);
		assert(check);
		err = ccs_release_object(configuration);
		assert(err == CCS_SUCCESS);
	}

	err = ccs_configuration_space_samples(space, 100, configurations);
	assert(err == CCS_SUCCESS);

	for (int i = 0; i < 100; i++) {
		ccs_float_t f;
		ccs_bool_t  check;
		err = ccs_configuration_get_values(
			configurations[i], 3, values, NULL);
		assert(err == CCS_SUCCESS);
		assert(values[1].type == CCS_FLOAT);
		f = values[1].value.f;
		assert(f >= -1.0 && f < 1.0);
		if (f < 0.0) {
			assert(values[2].type == CCS_FLOAT);
			f = values[2].value.f;
			assert(f >= -1.0 && f < 1.0);
			if (f < 0.0) {
				assert(values[0].type == CCS_FLOAT);
				f = values[0].value.f;
				assert(f >= 0.0 && f < 1.0);
			} else
				assert(values[0].type == CCS_INACTIVE);
		} else {
			assert(values[2].type == CCS_INACTIVE);
			assert(values[0].type == CCS_INACTIVE);
		}
		err = ccs_configuration_space_check_configuration(
			space, configurations[i], &check);
		assert(err == CCS_SUCCESS);
		assert(check);
		err = ccs_release_object(configurations[i]);
		assert(err == CCS_SUCCESS);
	}

	for (int i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_SUCCESS);
	}
	err = ccs_release_object(space);
	assert(err == CCS_SUCCESS);
}

int
main()
{
	ccs_init();
	test_simple();
	test_combined();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
