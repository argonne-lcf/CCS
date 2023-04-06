#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

#define NUM_SAMPLES 10000

static void compare_parameter(ccs_parameter_t parameter) {
	ccs_error_t               err;
	ccs_parameter_type_t  type;
	ccs_datum_t                default_value;
	const char                *name;
	ccs_distribution_t         distribution;
	ccs_distribution_type_t    dist_type;
	ccs_interval_t             interval;
	ccs_bool_t                 check;

	err = ccs_parameter_get_type(parameter, &type);
	assert( err == CCS_SUCCESS );
	assert( type == CCS_PARAMETER_TYPE_NUMERICAL );

	err = ccs_parameter_get_default_value(parameter, &default_value);
	assert( err == CCS_SUCCESS );
	assert( default_value.type == CCS_FLOAT );
	assert( default_value.value.f == 1.0 );

	err = ccs_parameter_get_name(parameter, &name);
	assert( err == CCS_SUCCESS );
	assert( strcmp(name, "my_param") == 0 );

	err = ccs_parameter_get_default_distribution(parameter, &distribution);
	assert( err == CCS_SUCCESS );
	assert( distribution );

	err = ccs_distribution_get_type(distribution, &dist_type);
	assert( err == CCS_SUCCESS );
	assert( dist_type == CCS_UNIFORM );

	err = ccs_distribution_get_bounds(distribution, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_FLOAT );
	assert( interval.lower.f  == -5.0 );
	assert( interval.lower_included == CCS_TRUE );
	assert( interval.upper.f  == 5.0 );
	assert( interval.upper_included == CCS_FALSE );

	err = ccs_parameter_check_value(parameter, ccs_float(1.0), &check);
	assert( err == CCS_SUCCESS );
	assert( check == CCS_TRUE );

	err = ccs_parameter_check_value(parameter, ccs_float(6.0), &check);
	assert( err == CCS_SUCCESS );
	assert( check == CCS_FALSE );

	err = ccs_release_object(distribution);
	assert( err == CCS_SUCCESS );
}

static void test_create() {
	ccs_parameter_t       parameter;
	ccs_error_t               err;
	char                      *buff;
	size_t                     buff_size;

	err = ccs_create_numerical_parameter("my_param", CCS_NUM_FLOAT,
	                                          CCSF(-5.0), CCSF(5.0),
	                                          CCSF(0.0), CCSF(1.0),
	                                          &parameter);
	assert( err == CCS_SUCCESS );

	compare_parameter(parameter);

	err = ccs_object_serialize(parameter, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE, &buff_size, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	buff = (char *)malloc(buff_size);
	assert( buff );

	err = ccs_object_serialize(parameter, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(parameter);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&parameter, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );
	free(buff);

	compare_parameter(parameter);

	err = ccs_release_object(parameter);
	assert( err == CCS_SUCCESS );
}

void test_samples() {
	ccs_rng_t                  rng;
	ccs_parameter_t       parameter;
	ccs_distribution_t         distribution;
	const size_t               num_samples = NUM_SAMPLES;
	ccs_datum_t                samples[NUM_SAMPLES];
	ccs_error_t               err;

	err = ccs_create_rng(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_numerical_parameter("my_param", CCS_NUM_FLOAT,
	                                          CCSF(-5.0), CCSF(5.0),
	                                          CCSF(0.0), CCSF(1.0),
	                                          &parameter);
	assert( err == CCS_SUCCESS );

	err = ccs_parameter_get_default_distribution(parameter, &distribution);
	assert( err == CCS_SUCCESS );
	assert( distribution );

	err = ccs_parameter_samples(parameter, distribution, rng,
	                                 num_samples, samples);
	assert( err == CCS_SUCCESS );

	for( size_t i = 0; i < num_samples; i++) {
		assert( samples[i].type == CCS_FLOAT );
		assert( samples[i].value.f >= -5.0 && samples[i].value.f < 5.0 );
	}

	err = ccs_release_object(distribution);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(parameter);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

void test_oversampling() {
	ccs_rng_t                  rng;
	ccs_parameter_t       parameter;
	ccs_distribution_t         distribution;
	const size_t               num_samples = NUM_SAMPLES;
	ccs_datum_t                samples[NUM_SAMPLES];
	ccs_error_t               err;

	err = ccs_create_rng(&rng);
	assert( err == CCS_SUCCESS );

	err = ccs_create_normal_float_distribution(0.0, 1.0, CCS_LINEAR, 0.0,
	                                           &distribution);
	assert( err == CCS_SUCCESS );

	err = ccs_create_numerical_parameter("my_param", CCS_NUM_FLOAT,
	                                          CCSF(-1.0), CCSF(1.0),
	                                          CCSF(0.0), CCSF(0.0),
	                                          &parameter);
	assert( err == CCS_SUCCESS );

	err = ccs_parameter_samples(parameter, distribution, rng,
	                                 num_samples, samples);
	assert( err == CCS_SUCCESS );

	for( size_t i = 0; i < num_samples; i++) {
		assert( samples[i].type == CCS_FLOAT );
		assert( samples[i].value.f >= -1.0 && samples[i].value.f < 1.0 );
	}

	err = ccs_release_object(distribution);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(parameter);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

int main() {
	ccs_init();
	test_create();
	test_samples();
	test_oversampling();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
