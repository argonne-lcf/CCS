#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>
#include <math.h>
#include "test_utils.h"

void
test_simple(void)
{
	ccs_parameter_t           parameter1, parameter2;
	ccs_parameter_t           parameters[2];
	ccs_configuration_space_t space;
	ccs_expression_t          expression;
	ccs_configuration_t       configuration;
	ccs_datum_t               values[2];
	ccs_configuration_t       configurations[100];
	ccs_result_t              err;

	parameters[0] = parameter1 = create_numerical("param1", -1.0, 1.0);
	parameters[1] = parameter2 = create_numerical("param2", -1.0, 1.0);
	err                        = ccs_create_binary_expression(
                CCS_EXPRESSION_TYPE_LESS, ccs_object(parameter1),
                ccs_float(0.0), &expression);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_configuration_space(
		"space", 2, parameters, NULL, 1, &expression, NULL, NULL,
		&space);
	assert(err == CCS_RESULT_SUCCESS);

	for (int i = 0; i < 100; i++) {
		ccs_float_t f;
		err = ccs_configuration_space_sample(
			space, NULL, NULL, NULL, &configuration);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_binding_get_values(
			(ccs_binding_t)configuration, 2, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		assert(values[0].type == CCS_DATA_TYPE_FLOAT);
		f = values[0].value.f;
		assert(f >= 0.0 && f < 1.0);
		assert(values[1].type == CCS_DATA_TYPE_FLOAT);
		f = values[0].value.f;
		assert(f >= -1.0 && f < 1.0);
		err = ccs_release_object(configuration);
		assert(err == CCS_RESULT_SUCCESS);
	}
	err = ccs_configuration_space_samples(
		space, NULL, NULL, NULL, 100, configurations);
	assert(err == CCS_RESULT_SUCCESS);

	for (int i = 0; i < 100; i++) {
		ccs_float_t f;
		err = ccs_binding_get_values(
			(ccs_binding_t)configurations[i], 2, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		assert(values[0].type == CCS_DATA_TYPE_FLOAT);
		f = values[0].value.f;
		assert(f >= 0.0 && f < 1.0);
		assert(values[1].type == CCS_DATA_TYPE_FLOAT);
		f = values[0].value.f;
		assert(f >= -1.0 && f < 1.0);
		err = ccs_release_object(configurations[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}

	err = ccs_release_object(expression);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter2);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(space);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_combined(void)
{
	ccs_parameter_t           parameters[3];
	ccs_expression_t          conditions[3] = {NULL, NULL, NULL};
	ccs_configuration_space_t space;
	ccs_expression_t          expression;
	ccs_configuration_t       configuration;
	ccs_datum_t               values[3];
	ccs_configuration_t       configurations[100];
	ccs_result_t              err;

	parameters[0] = create_numerical("param1", -1.0, 1.0);
	parameters[1] = create_numerical("param2", -1.0, 1.0);
	parameters[2] = create_numerical("param3", -1.0, 1.0);

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
		"space", 3, parameters, conditions, 1, &expression, NULL, NULL,
		&space);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(conditions[2]);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(conditions[0]);
	assert(err == CCS_RESULT_SUCCESS);

	for (int i = 0; i < 100; i++) {
		ccs_float_t f;
		err = ccs_configuration_space_sample(
			space, NULL, NULL, NULL, &configuration);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_binding_get_values(
			(ccs_binding_t)configuration, 3, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		assert(values[1].type == CCS_DATA_TYPE_FLOAT);
		f = values[1].value.f;
		assert(f >= -1.0 && f < 1.0);
		if (f < 0.0) {
			assert(values[2].type == CCS_DATA_TYPE_FLOAT);
			f = values[2].value.f;
			assert(f >= -1.0 && f < 1.0);
			if (f < 0.0) {
				assert(values[0].type == CCS_DATA_TYPE_FLOAT);
				f = values[0].value.f;
				assert(f >= 0.0 && f < 1.0);
			} else
				assert(values[0].type ==
				       CCS_DATA_TYPE_INACTIVE);
		} else {
			assert(values[2].type == CCS_DATA_TYPE_INACTIVE);
			assert(values[0].type == CCS_DATA_TYPE_INACTIVE);
		}
		err = ccs_release_object(configuration);
		assert(err == CCS_RESULT_SUCCESS);
	}

	err = ccs_configuration_space_samples(
		space, NULL, NULL, NULL, 100, configurations);
	assert(err == CCS_RESULT_SUCCESS);

	for (int i = 0; i < 100; i++) {
		ccs_float_t f;
		err = ccs_binding_get_values(
			(ccs_binding_t)configurations[i], 3, values, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		assert(values[1].type == CCS_DATA_TYPE_FLOAT);
		f = values[1].value.f;
		assert(f >= -1.0 && f < 1.0);
		if (f < 0.0) {
			assert(values[2].type == CCS_DATA_TYPE_FLOAT);
			f = values[2].value.f;
			assert(f >= -1.0 && f < 1.0);
			if (f < 0.0) {
				assert(values[0].type == CCS_DATA_TYPE_FLOAT);
				f = values[0].value.f;
				assert(f >= 0.0 && f < 1.0);
			} else
				assert(values[0].type ==
				       CCS_DATA_TYPE_INACTIVE);
		} else {
			assert(values[2].type == CCS_DATA_TYPE_INACTIVE);
			assert(values[0].type == CCS_DATA_TYPE_INACTIVE);
		}
		err = ccs_release_object(configurations[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}

	for (int i = 0; i < 3; i++) {
		err = ccs_release_object(parameters[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
	err = ccs_release_object(space);
	assert(err == CCS_RESULT_SUCCESS);
}

int
main(void)
{
	ccs_init();
	test_simple();
	test_combined();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
