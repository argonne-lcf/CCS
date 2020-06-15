#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>

static void test_create_uniform_distribution() {
	ccs_distribution_t      distrib = NULL;
	ccs_result_t            err = CCS_SUCCESS;
	int32_t                 refcount;
	ccs_object_type_t       otype;
	ccs_distribution_type_t dtype;
	ccs_scale_type_t        stype;
	ccs_numeric_type_t      data_type;
	ccs_numeric_t           quantization, lower, upper;
	ccs_interval_t          interval;
	ccs_int_t               l = -10L;
	ccs_int_t               u = 11L;
	ccs_int_t               q = 0L;

	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(l),
		CCSI(u),
		CCS_LINEAR,
		CCSI(q),
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
	assert( data_type == CCS_NUM_INTEGER );

	err = ccs_distribution_get_scale_type(distrib, &stype);
	assert( err == CCS_SUCCESS );
	assert( stype == CCS_LINEAR );

	err = ccs_distribution_get_quantization(distrib, &quantization);
	assert( err == CCS_SUCCESS );
	assert( quantization.i == q );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_INTEGER );
	assert( interval.lower.i == l );
	assert( interval.lower_included == CCS_TRUE );
	assert( interval.upper.i == u );
	assert( interval.upper_included == CCS_FALSE );

	err = ccs_uniform_distribution_get_parameters(distrib, &lower, &upper);
	assert( err == CCS_SUCCESS );
	assert( lower.i == l );
	assert( upper.i == u );

	err = ccs_object_get_refcount(distrib, &refcount);
	assert( err == CCS_SUCCESS );
	assert( refcount == 1 );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
}

static void test_create_uniform_distribution_errors() {
	ccs_distribution_t      distrib = NULL;
	ccs_result_t            err = CCS_SUCCESS;

	// check wrong data_type
	err = ccs_create_uniform_distribution(
		(ccs_numeric_type_t)CCS_STRING,
		CCSI(-10),
		CCSI(11),
		CCS_LINEAR,
		CCSI(0),
		&distrib);
	assert( err == -CCS_INVALID_TYPE );

	// check wrong data_type
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(-10),
		CCSI(11),
		(ccs_scale_type_t)3,
		CCSI(0),
		&distrib);
	assert( err == -CCS_INVALID_SCALE );

	// check wrong bounds
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(10),
		CCSI(-11),
		CCS_LINEAR,
		CCSI(0),
		&distrib);
	assert( err == -CCS_INVALID_VALUE );

	// check wrong quantization
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(-10),
		CCSI(11),
		CCS_LINEAR,
		CCSI(-1),
		&distrib);
	assert( err == -CCS_INVALID_VALUE );

	// check wrong pointer
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(10),
		CCSI(-11),
		CCS_LINEAR,
		CCSI(0),
		NULL);
	assert( err == -CCS_INVALID_VALUE );


}

static void test_uniform_distribution_int() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_samples = 100;
	ccs_int_t          lower = -10;
	ccs_int_t          upper = 11;
	ccs_numeric_t      samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(lower),
		CCSI(upper),
		CCS_LINEAR,
		CCSI(0),
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
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
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_samples = 1000;
	ccs_int_t          lower = 1;
	ccs_int_t          upper = 100;
	ccs_numeric_t      samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(lower),
		CCSI(upper),
		CCS_LOGARITHMIC,
		CCSI(0),
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
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
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_samples = 1000;
	ccs_int_t          lower = 1;
	ccs_int_t          upper = 101;
	ccs_int_t          quantize = 2;
	ccs_numeric_t      samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(lower),
		CCSI(upper),
		CCS_LOGARITHMIC,
		CCSI(quantize),
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
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
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_samples = 100;
	ccs_int_t          lower = -10;
	ccs_int_t          upper = 12;
	ccs_int_t          quantize = 2;
	ccs_numeric_t      samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(lower),
		CCSI(upper),
		CCS_LINEAR,
		CCSI(quantize),
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
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
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_samples = 100;
	ccs_float_t        lower = -10;
	ccs_float_t        upper = 11;
	ccs_numeric_t      samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(lower),
		CCSF(upper),
		CCS_LINEAR,
		CCSF(0.0),
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
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
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_samples = 1000;
	ccs_float_t        lower = 1;
	ccs_float_t        upper = 100;
	ccs_numeric_t      samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(lower),
		CCSF(upper),
		CCS_LOGARITHMIC,
		CCSF(0.0),
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
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
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_samples = 1000;
	ccs_float_t        lower = 1;
	ccs_float_t        upper = 101;
	ccs_float_t        quantize = 2;
	ccs_numeric_t      samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(lower),
		CCSF(upper),
		CCS_LOGARITHMIC,
		CCSF(quantize),
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
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
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_samples = 100;
	ccs_float_t        lower = -10;
	ccs_float_t        upper = 12;
	ccs_float_t        quantize = 2;
	ccs_numeric_t      samples[num_samples];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(lower),
		CCSF(upper),
		CCS_LINEAR,
		CCSF(quantize),
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
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
