#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>

static void test_create_uniform_distribution() {
	ccs_distribution_t      distrib = NULL;
	ccs_error_t             err = CCS_SUCCESS;
	int32_t                 refcount;
	size_t                  num_parameters;
	ccs_object_type_t       otype;
	ccs_distribution_type_t dtype;
	ccs_scale_type_t        stype;
	ccs_data_type_t         data_type;
	ccs_datum_t             parameters[2], quantization, lower, upper;

	err = ccs_create_uniform_distribution(
		CCS_INTEGER,
		-10L,
		11L,
		CCS_LINEAR,
		0L,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_object_get_type(distrib, &otype);
	assert( err == CCS_SUCCESS );
	assert( otype == CCS_DISTRIBUTION );

	err = ccs_distribution_get_type(distrib, &dtype);
	assert( err == CCS_SUCCESS );
	assert( dtype == CCS_UNIFORM );

	err = ccs_distribution_get_data_type(distrib, &data_type);
	assert( err == CCS_SUCCESS );
	assert( data_type == CCS_INTEGER );

	err = ccs_distribution_get_scale_type(distrib, &stype);
	assert( err == CCS_SUCCESS );
	assert( stype == CCS_LINEAR );

	err = ccs_distribution_get_quantization(distrib, &quantization);
	assert( err == CCS_SUCCESS );
	assert( quantization.type == CCS_INTEGER );
	assert( quantization.value.i == 0L );

	err = ccs_distribution_get_num_parameters(distrib, &num_parameters);
	assert( err == CCS_SUCCESS );
	assert( num_parameters == 2 );

	err = ccs_distribution_get_parameters(distrib, 2, parameters, &num_parameters);
	assert( err == CCS_SUCCESS );
	assert( num_parameters == 2 );
	assert( parameters[0].type == CCS_INTEGER );
	assert( parameters[0].value.i == -10L );
	assert( parameters[1].type == CCS_INTEGER );
	assert( parameters[1].value.i == 11L );

	err = ccs_uniform_distribution_get_parameters(distrib, &lower, &upper);
	assert( err == CCS_SUCCESS );
	assert( lower.type == CCS_INTEGER );
	assert( lower.value.i == -10L );
	assert( upper.type == CCS_INTEGER );
	assert( upper.value.i == 11L );

	err = ccs_object_get_refcount(distrib, &refcount);
	assert( err == CCS_SUCCESS );
	assert( refcount == 1 );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
}

static void test_create_uniform_distribution_errors() {
	ccs_distribution_t      distrib = NULL;
	ccs_error_t             err = CCS_SUCCESS;

	// check wrong data_type
	err = ccs_create_uniform_distribution(
		CCS_OBJECT,
		-10L,
		11L,
		CCS_LINEAR,
		0L,
		&distrib);
	assert( err == -CCS_INVALID_TYPE );

	// check wrong data_type
	err = ccs_create_uniform_distribution(
		CCS_INTEGER,
		-10L,
		11L,
		3,
		0L,
		&distrib);
	assert( err == -CCS_INVALID_SCALE );

	// check wrong bounds
	err = ccs_create_uniform_distribution(
		CCS_INTEGER,
		10L,
		-11L,
		CCS_LINEAR,
		0L,
		&distrib);
	assert( err == -CCS_INVALID_VALUE );

	// check wrong quantization
	err = ccs_create_uniform_distribution(
		CCS_INTEGER,
		-10L,
		11L,
		CCS_LINEAR,
		-1L,
		&distrib);
	assert( err == -CCS_INVALID_VALUE );

	// check wrong pointer
	err = ccs_create_uniform_distribution(
		CCS_INTEGER,
		10L,
		-11L,
		CCS_LINEAR,
		0L,
		NULL);
	assert( err == -CCS_INVALID_VALUE );


}

static void test_uniform_distribution_int() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 100;
	const ccs_int_t    lower = -10;
	const ccs_int_t    upper = 11;
	ccs_value_t        samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_INTEGER,
		lower,
		upper,
		CCS_LINEAR,
		0L,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (int i = 0; i < num_samples; i++) {
		assert(samples[i].i >= lower);
		assert(samples[i].i < upper);
	}

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_uniform_distribution_int_log() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 1000;
	const ccs_int_t    lower = 1;
	const ccs_int_t    upper = 100;
	ccs_value_t        samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_INTEGER,
		lower,
		upper,
		CCS_LOGARITHMIC,
		0L,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (int i = 0; i < num_samples; i++) {
		assert(samples[i].i >= lower);
		assert(samples[i].i < upper);
	}

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_uniform_distribution_int_log_quantize() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 1000;
	const ccs_int_t    lower = 1;
	const ccs_int_t    upper = 101;
	const ccs_int_t    quantize = 2;
	ccs_value_t        samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_INTEGER,
		lower,
		upper,
		CCS_LOGARITHMIC,
		quantize,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (int i = 0; i < num_samples; i++) {
		assert(samples[i].i >= lower);
		assert(samples[i].i < upper);
		assert((samples[i].i - lower) % quantize == 0);
	}

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_uniform_distribution_int_quantize() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 100;
	const ccs_int_t    lower = -10;
	const ccs_int_t    upper = 12;
	const ccs_int_t    quantize = 2;
	ccs_value_t        samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_INTEGER,
		lower,
		upper,
		CCS_LINEAR,
		quantize,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (int i = 0; i < num_samples; i++) {
		assert(samples[i].i >= lower);
		assert(samples[i].i < upper);
		assert((samples[i].i - lower) % quantize == 0);
	}

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_uniform_distribution_float() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 100;
	const ccs_float_t  lower = -10;
	const ccs_float_t  upper = 11;
	ccs_value_t        samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_FLOAT,
		lower,
		upper,
		CCS_LINEAR,
		0.0,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (int i = 0; i < num_samples; i++) {
		assert(samples[i].f >= lower);
		assert(samples[i].f < upper);
	}

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_uniform_distribution_float_log() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 1000;
	const ccs_float_t  lower = 1;
	const ccs_float_t  upper = 100;
	ccs_value_t        samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_FLOAT,
		lower,
		upper,
		CCS_LOGARITHMIC,
		0.0,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (int i = 0; i < num_samples; i++) {
		assert(samples[i].f >= lower);
		assert(samples[i].f < upper);
	}

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_uniform_distribution_float_log_quantize() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 1000;
	const ccs_float_t  lower = 1;
	const ccs_float_t  upper = 101;
	const ccs_float_t  quantize = 2;
	ccs_value_t        samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_FLOAT,
		lower,
		upper,
		CCS_LOGARITHMIC,
		quantize,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (int i = 0; i < num_samples; i++) {
		assert(samples[i].f >= lower);
		assert(samples[i].f < upper);
	}

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_uniform_distribution_float_quantize() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 100;
	const ccs_float_t  lower = -10;
	const ccs_float_t  upper = 12;
	const ccs_float_t  quantize = 2;
	ccs_value_t        samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_FLOAT,
		lower,
		upper,
		CCS_LINEAR,
		quantize,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (int i = 0; i < num_samples; i++) {
		assert(samples[i].f >= lower);
		assert(samples[i].f < upper);
	}

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

int main(int argc, char *argv[]) {
	ccs_init();
	test_create_uniform_distribution();
	test_create_uniform_distribution_errors();
	test_uniform_distribution_int();
	test_uniform_distribution_int_log();
	test_uniform_distribution_int_quantize();
	test_uniform_distribution_int_log_quantize();
	test_uniform_distribution_float();
	test_uniform_distribution_float_log();
	test_uniform_distribution_float_quantize();
	test_uniform_distribution_float_log_quantize();
	return 0;
}
