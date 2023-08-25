#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

#define NUM_POSSIBLE_VALUES 4
#define NUM_SAMPLES         10000

static void
compare_parameter(
	ccs_parameter_t parameter,
	const size_t    num_possible_values,
	ccs_datum_t     possible_values[],
	const size_t    default_value_index)
{
	ccs_result_t            err;
	ccs_parameter_type_t    type;
	ccs_datum_t             default_value;
	const char             *name;
	ccs_distribution_t      distribution;
	ccs_distribution_type_t dist_type;
	ccs_interval_t          interval;
	ccs_bool_t              check;

	err = ccs_parameter_get_type(parameter, &type);
	assert(err == CCS_RESULT_SUCCESS);
	assert(type == CCS_PARAMETER_TYPE_DISCRETE);

	err = ccs_parameter_get_default_value(parameter, &default_value);
	assert(err == CCS_RESULT_SUCCESS);
	assert(default_value.type == CCS_DATA_TYPE_INT);
	assert(default_value.value.i ==
	       possible_values[default_value_index].value.i);

	err = ccs_parameter_get_name(parameter, &name);
	assert(err == CCS_RESULT_SUCCESS);
	assert(strcmp(name, "my_param") == 0);

	err = ccs_parameter_get_default_distribution(parameter, &distribution);
	assert(err == CCS_RESULT_SUCCESS);
	assert(distribution);

	err = ccs_distribution_get_type(distribution, &dist_type);
	assert(err == CCS_RESULT_SUCCESS);
	assert(dist_type == CCS_DISTRIBUTION_TYPE_UNIFORM);

	err = ccs_distribution_get_bounds(distribution, &interval);
	assert(err == CCS_RESULT_SUCCESS);
	assert(interval.type == CCS_NUMERIC_TYPE_INT);
	assert(interval.lower.i == 0);
	assert(interval.lower_included == CCS_TRUE);
	assert(interval.upper.i == 4);
	assert(interval.upper_included == CCS_FALSE);

	for (size_t i = 0; i < num_possible_values; i++) {
		err = ccs_parameter_check_value(
			parameter, possible_values[i], &check);
		assert(err == CCS_RESULT_SUCCESS);
		assert(check == CCS_TRUE);
	}

	default_value.type = CCS_DATA_TYPE_FLOAT;
	err = ccs_parameter_check_value(parameter, default_value, &check);
	assert(err == CCS_RESULT_SUCCESS);
	assert(check == CCS_FALSE);

	err = ccs_release_object(distribution);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_create(void)
{
	ccs_parameter_t parameter;
	ccs_result_t    err;
	const size_t    num_possible_values = NUM_POSSIBLE_VALUES;
	ccs_datum_t     possible_values[NUM_POSSIBLE_VALUES];
	const size_t    default_value_index = 2;
	char           *buff;
	size_t          buff_size;

	for (size_t i = 0; i < num_possible_values; i++) {
		possible_values[i].type    = CCS_DATA_TYPE_INT;
		possible_values[i].value.i = (i + 1) * 2;
	}

	err = ccs_create_discrete_parameter(
		"my_param", num_possible_values, possible_values,
		default_value_index, &parameter);
	assert(err == CCS_RESULT_SUCCESS);

	compare_parameter(
		parameter, num_possible_values, possible_values,
		default_value_index);

	err = ccs_object_serialize(
		parameter, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		parameter, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(parameter);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&parameter, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	free(buff);

	compare_parameter(
		parameter, num_possible_values, possible_values,
		default_value_index);

	err = ccs_release_object(parameter);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_samples(void)
{
	ccs_rng_t          rng;
	ccs_parameter_t    parameter;
	ccs_distribution_t distribution;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_datum_t        samples[NUM_SAMPLES];
	ccs_result_t       err;
	const size_t       num_possible_values = NUM_POSSIBLE_VALUES;
	ccs_datum_t        possible_values[NUM_POSSIBLE_VALUES];
	const size_t       default_value_index = 2;

	for (size_t i = 0; i < num_possible_values; i++) {
		possible_values[i].type    = CCS_DATA_TYPE_INT;
		possible_values[i].value.i = (i + 1) * 2;
	}

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_discrete_parameter(
		"my_param", num_possible_values, possible_values,
		default_value_index, &parameter);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_parameter_get_default_distribution(parameter, &distribution);
	assert(err == CCS_RESULT_SUCCESS);
	assert(distribution);

	err = ccs_parameter_samples(
		parameter, distribution, rng, num_samples, samples);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t i = 0; i < num_samples; i++) {
		assert(samples[i].type == CCS_DATA_TYPE_INT);
		assert(samples[i].value.i % 2 == 0);
		assert(samples[i].value.i >= 2);
		assert(samples[i].value.i <=
		       (ccs_int_t)num_possible_values * 2);
	}

	err = ccs_release_object(distribution);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_oversampling(void)
{
	ccs_rng_t          rng;
	ccs_parameter_t    parameter;
	ccs_distribution_t distribution;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_datum_t        samples[NUM_SAMPLES];
	ccs_result_t       err;
	const size_t       num_possible_values = NUM_POSSIBLE_VALUES;
	ccs_datum_t        possible_values[NUM_POSSIBLE_VALUES];
	const size_t       default_value_index = 2;

	for (size_t i = 0; i < num_possible_values; i++) {
		possible_values[i].type    = CCS_DATA_TYPE_INT;
		possible_values[i].value.i = (i + 1) * 2;
	}

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_uniform_int_distribution(
		0, num_possible_values + 1, CCS_SCALE_TYPE_LINEAR, 0,
		&distribution);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_discrete_parameter(
		"my_param", num_possible_values, possible_values,
		default_value_index, &parameter);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_parameter_samples(
		parameter, distribution, rng, num_samples, samples);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t i = 0; i < num_samples; i++) {
		assert(samples[i].type == CCS_DATA_TYPE_INT);
		assert(samples[i].value.i % 2 == 0);
		assert(samples[i].value.i >= 2);
		assert(samples[i].value.i <=
		       (ccs_int_t)num_possible_values * 2);
	}

	err = ccs_release_object(distribution);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

int
main(void)
{
	ccs_init();
	test_create();
	test_samples();
	test_oversampling();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
