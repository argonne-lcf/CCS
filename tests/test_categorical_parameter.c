#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <cconfigspace.h>
#include <string.h>
#include "test_utils.h"

#define NUM_POSSIBLE_VALUES 4
#define NUM_SAMPLES         10000

void
free_data(ccs_object_t o, void *user_data)
{
	(void)o;
	free(user_data);
}

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
	void                   *user_data;
	ccs_distribution_t      distribution;
	ccs_distribution_type_t dist_type;
	ccs_interval_t          interval;
	ccs_bool_t              check;

	err = ccs_parameter_get_type(parameter, &type);
	assert(err == CCS_RESULT_SUCCESS);
	assert(type == CCS_PARAMETER_TYPE_CATEGORICAL);

	err = ccs_parameter_get_default_value(parameter, &default_value);
	assert(err == CCS_RESULT_SUCCESS);
	assert(default_value.type == CCS_DATA_TYPE_INT);
	assert(default_value.value.i ==
	       possible_values[default_value_index].value.i);

	err = ccs_parameter_get_name(parameter, &name);
	assert(err == CCS_RESULT_SUCCESS);
	assert(strcmp(name, "my_param") == 0);

	err = ccs_object_get_user_data(parameter, &user_data);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!strcmp((char *)user_data, "hello"));

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

ccs_result_t
serialize_callback(
	ccs_object_t object,
	size_t       serialize_data_size,
	void        *serialize_data,
	size_t      *serialize_data_size_ret,
	void        *callback_user_data)
{
	void  *user_data;
	size_t sz;
	assert(callback_user_data == (void *)0xdeadbeef);
	ccs_result_t err = ccs_object_get_user_data(object, &user_data);
	assert(err == CCS_RESULT_SUCCESS);
	assert(user_data);
	sz = strlen((char *)user_data) + 1;
	if (serialize_data_size_ret)
		*serialize_data_size_ret = sz;
	assert(!(serialize_data && serialize_data_size < sz));
	if (serialize_data)
		strncpy((char *)serialize_data, (char *)user_data, sz);
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
deserialize_callback(
	ccs_object_t object,
	size_t       serialize_data_size,
	void        *serialize_data,
	void        *callback_user_data)
{
	assert(callback_user_data == (void *)0xbeefdead);
	assert(strlen((char *)serialize_data) + 1 == serialize_data_size);
	void        *user_data = (void *)strdup((char *)serialize_data);
	ccs_result_t err       = ccs_object_set_user_data(object, user_data);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_object_set_destroy_callback(object, &free_data, user_data);
	assert(err == CCS_RESULT_SUCCESS);
	return CCS_RESULT_SUCCESS;
}

void
test_create(void)
{
	ccs_parameter_t parameter;
	ccs_result_t    err;
	void           *user_data;
	const size_t    num_possible_values = NUM_POSSIBLE_VALUES;
	ccs_datum_t     possible_values[NUM_POSSIBLE_VALUES];
	const size_t    default_value_index = 2;
	char           *buff;
	size_t          buff_size;

	for (size_t i = 0; i < num_possible_values; i++) {
		possible_values[i].type    = CCS_DATA_TYPE_INT;
		possible_values[i].value.i = (i + 1) * 2;
	}

	user_data = (void *)strdup("hello");

	err       = ccs_create_categorical_parameter(
                "my_param", num_possible_values, possible_values,
                default_value_index, &parameter);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_set_user_data(parameter, user_data);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_set_destroy_callback(parameter, &free_data, user_data);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_set_serialize_callback(
		parameter, serialize_callback, (void *)0xdeadbeef);
	assert(err == CCS_RESULT_SUCCESS);

	compare_parameter(
		parameter, num_possible_values, possible_values,
		default_value_index);

	err = ccs_object_serialize(
		parameter, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_CALLBACK, serialize_callback,
		(void *)0xdeadbeef, CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		parameter, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_CALLBACK, serialize_callback,
		(void *)0xdeadbeef, CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(parameter);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&parameter, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_DESERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_DATA_CALLBACK, deserialize_callback,
		(void *)0xbeefdead, CCS_DESERIALIZE_OPTION_END);
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
	err = ccs_create_categorical_parameter(
		"my_param", num_possible_values, possible_values,
		default_value_index, &parameter);
	assert(err == CCS_RESULT_SUCCESS);

	{
		ccs_serialize_format_t formats[] = {
			CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_FORMAT_JSON};
		size_t num_formats = sizeof(formats) / sizeof(formats[0]);
		for (size_t f = 0; f < num_formats; f++) {
			ccs_parameter_t      parameter2;
			ccs_parameter_type_t ptype;
			const char          *pname;

			test_serialize_deserialize(
				(ccs_object_t)parameter, formats[f],
				(ccs_object_t *)&parameter2);
			err = ccs_parameter_get_type(parameter2, &ptype);
			assert(err == CCS_RESULT_SUCCESS);
			assert(ptype == CCS_PARAMETER_TYPE_CATEGORICAL);
			err = ccs_parameter_get_name(parameter2, &pname);
			assert(err == CCS_RESULT_SUCCESS);
			assert(!strcmp(pname, "my_param"));
			err = ccs_release_object(parameter2);
			assert(err == CCS_RESULT_SUCCESS);
		}
	}

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

	err = ccs_create_categorical_parameter(
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

static void
test_default_distribution(void)
{
	ccs_parameter_t         parameter    = NULL;
	ccs_distribution_t      distribution = NULL;
	ccs_result_t            err;
	ccs_datum_t             possible_values[4];
	ccs_distribution_type_t dtype;

	for (size_t i = 0; i < 4; i++) {
		possible_values[i].type    = CCS_DATA_TYPE_INT;
		possible_values[i].value.i = (ccs_int_t)(i + 1) * 2;
	}

	err = ccs_create_categorical_parameter(
		"my_param", 4, possible_values, 0, &parameter);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_parameter_get_default_distribution(parameter, &distribution);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_get_type(distribution, &dtype);
	assert(err == CCS_RESULT_SUCCESS);
	assert(dtype == CCS_DISTRIBUTION_TYPE_UNIFORM);

	err = ccs_release_object(distribution);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_diverse_values(void)
{
	ccs_parameter_t parameter;
	ccs_result_t    err;
	ccs_datum_t     possible_values[7];
	size_t          num_possible_values = 7;

	/* int */
	possible_values[0]                  = ccs_int(42);
	/* float (finite) */
	possible_values[1]                  = ccs_float(3.14);
	/* float (+Infinity) */
	possible_values[2]                  = ccs_float(INFINITY);
	/* float (-Infinity) */
	possible_values[3]                  = ccs_float(-INFINITY);
	/* float (NaN) */
	possible_values[4]                  = ccs_float(NAN);
	/* string */
	possible_values[5]                  = ccs_string("hello");
	/* bool */
	possible_values[6]                  = ccs_bool(CCS_TRUE);

	err                                 = ccs_create_categorical_parameter(
                "diverse", num_possible_values, possible_values, 0, &parameter);
	assert(err == CCS_RESULT_SUCCESS);

	{
		ccs_serialize_format_t formats[] = {
			CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_FORMAT_JSON};
		size_t num_formats = sizeof(formats) / sizeof(formats[0]);
		for (size_t f = 0; f < num_formats; f++) {
			ccs_parameter_t      parameter2;
			ccs_parameter_type_t ptype;
			const char          *pname;
			ccs_datum_t          dval;
			size_t               num_pv;

			test_serialize_deserialize(
				(ccs_object_t)parameter, formats[f],
				(ccs_object_t *)&parameter2);

			err = ccs_parameter_get_type(parameter2, &ptype);
			assert(err == CCS_RESULT_SUCCESS);
			assert(ptype == CCS_PARAMETER_TYPE_CATEGORICAL);

			err = ccs_parameter_get_name(parameter2, &pname);
			assert(err == CCS_RESULT_SUCCESS);
			assert(!strcmp(pname, "diverse"));

			err = ccs_categorical_parameter_get_values(
				parameter2, 0, NULL, &num_pv);
			assert(err == CCS_RESULT_SUCCESS);
			assert(num_pv == num_possible_values);

			/* default is index 0 = int(42) */
			err = ccs_parameter_get_default_value(
				parameter2, &dval);
			assert(err == CCS_RESULT_SUCCESS);
			assert(dval.type == CCS_DATA_TYPE_INT);
			assert(dval.value.i == 42);

			/* check all values are valid */
			for (size_t i = 0; i < num_possible_values; i++) {
				ccs_bool_t check;
				err = ccs_parameter_check_value(
					parameter2, possible_values[i], &check);
				assert(err == CCS_RESULT_SUCCESS);
				assert(check == CCS_TRUE);
			}

			err = ccs_release_object(parameter2);
			assert(err == CCS_RESULT_SUCCESS);
		}
	}

	err = ccs_release_object(parameter);
	assert(err == CCS_RESULT_SUCCESS);
}

int
main(void)
{
	ccs_init();
	test_create();
	test_samples();
	test_oversampling();
	test_default_distribution();
	test_diverse_values();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
