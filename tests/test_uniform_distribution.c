#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>

#define NUM_SAMPLES 100
#define NUM_SAMPLES_BIG 1000

static void compare_distribution(
		ccs_distribution_t distrib,
		ccs_int_t l, ccs_int_t u, ccs_int_t q) {
	ccs_error_t            err = CCS_SUCCESS;
	int32_t                 refcount;
	ccs_object_type_t       otype;
	ccs_distribution_type_t dtype;
	ccs_scale_type_t        stype;
	ccs_numeric_type_t      data_type;
	ccs_numeric_t           quantization, lower, upper;
	ccs_interval_t          interval;

	err = ccs_object_get_type(distrib, &otype);
	assert( err == CCS_SUCCESS );
	assert( otype == CCS_DISTRIBUTION );

	err = ccs_distribution_get_type(distrib, &dtype);
	assert( err == CCS_SUCCESS );
	assert( dtype == CCS_UNIFORM );

	err = ccs_distribution_get_data_types(distrib, &data_type);
	assert( err == CCS_SUCCESS );
	assert( data_type == CCS_NUM_INTEGER );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_INTEGER );
	assert( interval.lower.i == l );
	assert( interval.lower_included == CCS_TRUE );
	assert( interval.upper.i == u );
	assert( interval.upper_included == CCS_FALSE );

	err = ccs_uniform_distribution_get_properties(distrib, &lower, &upper, &stype, &quantization);
	assert( err == CCS_SUCCESS );
	assert( lower.i == l );
	assert( upper.i == u );
	assert( stype == CCS_LINEAR );
	assert( quantization.i == q );

	err = ccs_object_get_refcount(distrib, &refcount);
	assert( err == CCS_SUCCESS );
	assert( refcount == 1 );
}

static void test_create_uniform_distribution() {
	ccs_distribution_t  distrib = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	ccs_int_t           l = -10L;
	ccs_int_t           u = 11L;
	ccs_int_t           q = 0L;
	char               *buff;
	size_t              buff_size;

	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(l),
		CCSI(u),
		CCS_LINEAR,
		CCSI(q),
		&distrib);
	assert( err == CCS_SUCCESS );

	compare_distribution(distrib, l, u, q);

	err = ccs_object_serialize(distrib, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE, &buff_size, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	buff = (char *)malloc(buff_size);
	assert( buff );

	err = ccs_object_serialize(distrib, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&distrib, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );
	free(buff);

	compare_distribution(distrib, l, u, q);

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
}

static void test_create_uniform_distribution_errors() {
	ccs_distribution_t      distrib = NULL;
	ccs_error_t            err = CCS_SUCCESS;

	// check wrong data_type
	err = ccs_create_uniform_distribution(
		(ccs_numeric_type_t)CCS_STRING,
		CCSI(-10),
		CCSI(11),
		CCS_LINEAR,
		CCSI(0),
		&distrib);
	assert( err == CCS_INVALID_TYPE );

	// check wrong data_type
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(-10),
		CCSI(11),
		(ccs_scale_type_t)3,
		CCSI(0),
		&distrib);
	assert( err == CCS_INVALID_SCALE );

	// check wrong bounds
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(10),
		CCSI(-11),
		CCS_LINEAR,
		CCSI(0),
		&distrib);
	assert( err == CCS_INVALID_VALUE );

	// check wrong quantization
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(-10),
		CCSI(11),
		CCS_LINEAR,
		CCSI(-1),
		&distrib);
	assert( err == CCS_INVALID_VALUE );

	// check wrong pointer
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(10),
		CCSI(-11),
		CCS_LINEAR,
		CCSI(0),
		NULL);
	assert( err == CCS_INVALID_VALUE );


}

static void test_uniform_distribution_int() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t       err = CCS_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_int_t          lower = -10;
	ccs_int_t          upper = 11;
	ccs_numeric_t      samples[NUM_SAMPLES];

	err = ccs_create_rng(&rng);
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
	ccs_error_t       err = CCS_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES_BIG;
	ccs_int_t          lower = 1;
	ccs_int_t          upper = 100;
	ccs_numeric_t      samples[NUM_SAMPLES_BIG];

	err = ccs_create_rng(&rng);
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
	ccs_error_t       err = CCS_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES_BIG;
	ccs_int_t          lower = 1;
	ccs_int_t          upper = 101;
	ccs_int_t          quantize = 2;
	ccs_numeric_t      samples[NUM_SAMPLES_BIG];

	err = ccs_create_rng(&rng);
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
	ccs_error_t       err = CCS_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_int_t          lower = -10;
	ccs_int_t          upper = 12;
	ccs_int_t          quantize = 2;
	ccs_numeric_t      samples[NUM_SAMPLES];

	err = ccs_create_rng(&rng);
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
	ccs_error_t       err = CCS_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_float_t        lower = -10;
	ccs_float_t        upper = 11;
	ccs_numeric_t      samples[NUM_SAMPLES];

	err = ccs_create_rng(&rng);
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
	ccs_error_t       err = CCS_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES_BIG;
	ccs_float_t        lower = 1;
	ccs_float_t        upper = 100;
	ccs_numeric_t      samples[NUM_SAMPLES_BIG];

	err = ccs_create_rng(&rng);
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
	ccs_error_t       err = CCS_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES_BIG;
	ccs_float_t        lower = 1;
	ccs_float_t        upper = 101;
	ccs_float_t        quantize = 2;
	ccs_numeric_t      samples[NUM_SAMPLES_BIG];

	err = ccs_create_rng(&rng);
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
	ccs_error_t       err = CCS_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_float_t        lower = -10;
	ccs_float_t        upper = 12;
	ccs_float_t        quantize = 2;
	ccs_numeric_t      samples[NUM_SAMPLES];

	err = ccs_create_rng(&rng);
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

static void test_uniform_distribution_strided_samples() {
	ccs_distribution_t distrib1 = NULL;
	ccs_distribution_t distrib2 = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t       err = CCS_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_int_t          lower1 = -10;
	ccs_int_t          upper1 = 11;
	ccs_int_t          lower2 = 12;
	ccs_int_t          upper2 = 20;
	ccs_numeric_t      samples[NUM_SAMPLES*2];

	err = ccs_create_rng(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(lower1),
		CCSI(upper1),
		CCS_LINEAR,
		CCSI(0),
		&distrib1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(lower2),
		CCSI(upper2),
		CCS_LINEAR,
		CCSI(0),
		&distrib2);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_strided_samples(distrib1, rng, num_samples, 2, samples);
	err = ccs_distribution_strided_samples(distrib2, rng, num_samples, 2, &(samples[0])+1);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
		assert(samples[i*2].i >= lower1);
		assert(samples[i*2].i < upper1);
	}

	for (size_t i = 0; i < num_samples; i++) {
		assert(samples[i*2+1].i >= lower2);
		assert(samples[i*2+1].i < upper2);
	}

	err = ccs_release_object(distrib1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(distrib2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_uniform_distribution_soa_samples() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t       err = CCS_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_float_t        lower = -10;
	ccs_float_t        upper = 11;
	ccs_numeric_t      samples[NUM_SAMPLES];
	ccs_numeric_t     *p_samples;

	err = ccs_create_rng(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(lower),
		CCSF(upper),
		CCS_LINEAR,
		CCSF(0.0),
		&distrib);
	assert( err == CCS_SUCCESS );

        p_samples = &(samples[0]);
	err = ccs_distribution_soa_samples(distrib, rng, num_samples, &p_samples);
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


int main() {
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
	test_uniform_distribution_strided_samples();
	test_uniform_distribution_soa_samples();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
