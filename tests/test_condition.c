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
	ccs_expression_t          conditions[2] = {NULL, NULL};
	ccs_configuration_space_t space;
	ccs_configuration_t       configuration;
	ccs_datum_t               values[2];
	ccs_configuration_t       configurations[100];
	ccs_result_t              err;

	parameters[0] = parameter1 = create_numerical("param1", -1.0, 1.0);
	parameters[1] = parameter2 = create_numerical("param2", -1.0, 1.0);
	err                        = ccs_create_binary_expression(
                CCS_EXPRESSION_TYPE_LESS, ccs_object(parameter1),
                ccs_float(0.0), &conditions[1]);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_configuration_space(
		"space", 2, parameters, conditions, 0, NULL, NULL, NULL,
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
		assert(f >= -1.0 && f < 1.0);
		if (f < 0.0)
			assert(values[1].type == CCS_DATA_TYPE_FLOAT);
		else
			assert(values[1].type == CCS_DATA_TYPE_INACTIVE);
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
		assert(f >= -1.0 && f < 1.0);
		if (f < 0.0)
			assert(values[1].type == CCS_DATA_TYPE_FLOAT);
		else
			assert(values[1].type == CCS_DATA_TYPE_INACTIVE);
		err = ccs_release_object(configurations[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}

	err = ccs_release_object(conditions[1]);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter2);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(space);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_transitive(void)
{
	ccs_parameter_t           parameters[3];
	ccs_expression_t          conditions[3] = {NULL, NULL, NULL};
	ccs_configuration_space_t space;
	ccs_configuration_t       configuration;
	ccs_datum_t               values[3];
	ccs_configuration_t       configurations[100];
	ccs_result_t              err;

	parameters[0] = create_numerical("param1", -1.0, 1.0);
	parameters[1] = create_numerical("param2", -1.0, 1.0);
	parameters[2] = create_numerical("param3", -1.0, 1.0);

	err           = ccs_create_binary_expression(
                CCS_EXPRESSION_TYPE_LESS, ccs_object(parameters[1]),
                ccs_float(0.0), &conditions[2]);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_binary_expression(
		CCS_EXPRESSION_TYPE_LESS, ccs_object(parameters[2]),
		ccs_float(0.0), &conditions[0]);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_configuration_space(
		"space", 3, parameters, conditions, 0, NULL, NULL, NULL,
		&space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(conditions[0]);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(conditions[2]);
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
				assert(f >= -1.0 && f < 1.0);
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
				assert(f >= -1.0 && f < 1.0);
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

void
test_serialize_inactive(void)
{
	ccs_parameter_t           parameter1, parameter2;
	ccs_parameter_t           parameters[2];
	ccs_expression_t          conditions[2] = {NULL, NULL};
	ccs_configuration_space_t space;
	ccs_configuration_t       configuration;
	ccs_datum_t               values[2];
	ccs_result_t              err;

	parameters[0] = parameter1 = create_numerical("param1", -1.0, 1.0);
	parameters[1] = parameter2 = create_numerical("param2", -1.0, 1.0);
	err                        = ccs_create_binary_expression(
                CCS_EXPRESSION_TYPE_LESS, ccs_object(parameter1),
                ccs_float(0.0), &conditions[1]);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_configuration_space(
		"space", 2, parameters, conditions, 0, NULL, NULL, NULL,
		&space);
	assert(err == CCS_RESULT_SUCCESS);

	/* Create a configuration where param1 >= 0 so param2 is inactive */
	values[0] = ccs_float(0.5);
	values[1] = ccs_inactive;
	err = ccs_create_configuration(space, NULL, 2, values, &configuration);
	assert(err == CCS_RESULT_SUCCESS);

	{
		ccs_serialize_format_t formats[] = {
			CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_FORMAT_JSON};
		size_t num_formats = sizeof(formats) / sizeof(formats[0]);
		for (size_t f = 0; f < num_formats; f++) {
			ccs_configuration_space_t space2;
			ccs_configuration_t       config2;
			ccs_datum_t               vals2[2];
			ccs_map_t                 map;
			char                     *buff;
			size_t                    buff_size;

			err = ccs_create_map(&map);
			assert(err == CCS_RESULT_SUCCESS);

			err = ccs_object_serialize(
				space, formats[f],
				CCS_SERIALIZE_OPERATION_BUFFER, &buff,
				&buff_size, CCS_SERIALIZE_OPTION_END);
			assert(err == CCS_RESULT_SUCCESS);
			err = ccs_object_deserialize(
				(ccs_object_t *)&space2, formats[f],
				CCS_DESERIALIZE_OPERATION_MEMORY, buff_size,
				buff, CCS_DESERIALIZE_OPTION_MAP_HANDLES,
				CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
				CCS_DESERIALIZE_OPTION_END);
			assert(err == CCS_RESULT_SUCCESS);
			err = ccs_release_buffer(buff);
			assert(err == CCS_RESULT_SUCCESS);

			err = ccs_object_serialize(
				configuration, formats[f],
				CCS_SERIALIZE_OPERATION_BUFFER, &buff,
				&buff_size, CCS_SERIALIZE_OPTION_END);
			assert(err == CCS_RESULT_SUCCESS);
			err = ccs_object_deserialize(
				(ccs_object_t *)&config2, formats[f],
				CCS_DESERIALIZE_OPERATION_MEMORY, buff_size,
				buff, CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
				CCS_DESERIALIZE_OPTION_END);
			assert(err == CCS_RESULT_SUCCESS);

			err = ccs_binding_get_values(
				(ccs_binding_t)config2, 2, vals2, NULL);
			assert(err == CCS_RESULT_SUCCESS);
			assert(vals2[0].type == CCS_DATA_TYPE_FLOAT);
			assert(vals2[0].value.f == 0.5);
			assert(vals2[1].type == CCS_DATA_TYPE_INACTIVE);

			err = ccs_release_buffer(buff);
			assert(err == CCS_RESULT_SUCCESS);
			err = ccs_release_object(config2);
			assert(err == CCS_RESULT_SUCCESS);
			err = ccs_release_object(map);
			assert(err == CCS_RESULT_SUCCESS);
			err = ccs_release_object(space2);
			assert(err == CCS_RESULT_SUCCESS);
		}
	}

	err = ccs_release_object(configuration);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(conditions[1]);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter2);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(space);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_create_valid_invalid(void)
{
	ccs_parameter_t           parameter1, parameter2;
	ccs_parameter_t           parameters[2];
	ccs_expression_t          conditions[2] = {NULL, NULL};
	ccs_configuration_space_t space;
	ccs_configuration_t       configuration;
	ccs_datum_t               values[2];
	ccs_result_t              err;

	/* condition: param2 is active only when param1 < 0 */
	parameters[0] = parameter1 = create_numerical("param1", -1.0, 1.0);
	parameters[1] = parameter2 = create_numerical("param2", -1.0, 1.0);
	err                        = ccs_create_binary_expression(
                CCS_EXPRESSION_TYPE_LESS, ccs_object(parameter1),
                ccs_float(0.0), &conditions[1]);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_configuration_space(
		"space", 2, parameters, conditions, 0, NULL, NULL, NULL,
		&space);
	assert(err == CCS_RESULT_SUCCESS);

	/* Valid: param1 < 0, param2 active */
	values[0] = ccs_float(-0.5);
	values[1] = ccs_float(0.3);
	err = ccs_create_configuration(space, NULL, 2, values, &configuration);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(configuration);
	assert(err == CCS_RESULT_SUCCESS);

	/* Valid: param1 >= 0, param2 inactive */
	values[0] = ccs_float(0.5);
	values[1] = ccs_inactive;
	err = ccs_create_configuration(space, NULL, 2, values, &configuration);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(configuration);
	assert(err == CCS_RESULT_SUCCESS);

	/* Invalid: param1 < 0 but param2 inactive (should be active) */
	values[0] = ccs_float(-0.5);
	values[1] = ccs_inactive;
	err = ccs_create_configuration(space, NULL, 2, values, &configuration);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_clear_thread_error();

	/* Invalid: param1 >= 0 but param2 active (should be inactive) */
	values[0] = ccs_float(0.5);
	values[1] = ccs_float(0.3);
	err = ccs_create_configuration(space, NULL, 2, values, &configuration);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_clear_thread_error();

	/* Invalid: param1 in range but param2 out of range */
	values[0] = ccs_float(-0.5);
	values[1] = ccs_float(5.0);
	err = ccs_create_configuration(space, NULL, 2, values, &configuration);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_clear_thread_error();

	/* Invalid: param1 out of range */
	values[0] = ccs_float(5.0);
	values[1] = ccs_inactive;
	err = ccs_create_configuration(space, NULL, 2, values, &configuration);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_clear_thread_error();

	/* Invalid: both inactive */
	values[0] = ccs_inactive;
	values[1] = ccs_inactive;
	err = ccs_create_configuration(space, NULL, 2, values, &configuration);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_clear_thread_error();

	/* Invalid: param1 inactive (unconditional parameter) */
	values[0] = ccs_inactive;
	values[1] = ccs_float(0.3);
	err = ccs_create_configuration(space, NULL, 2, values, &configuration);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_clear_thread_error();

	/* Invalid: param1 out of range, param2 active */
	values[0] = ccs_float(-5.0);
	values[1] = ccs_float(0.3);
	err = ccs_create_configuration(space, NULL, 2, values, &configuration);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_clear_thread_error();

	err = ccs_release_object(conditions[1]);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter2);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(space);
	assert(err == CCS_RESULT_SUCCESS);
}

int
main(void)
{
	ccs_init();
	test_simple();
	test_transitive();
	test_serialize_inactive();
	test_create_valid_invalid();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
