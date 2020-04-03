#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>

static void test_create_normal_distribution() {
	ccs_distribution_t      distrib = NULL;
	ccs_error_t             err = CCS_SUCCESS;
	int32_t                 refcount;
	ccs_object_type_t       otype;
	ccs_distribution_type_t dtype;
	ccs_scale_type_t        stype;
	ccs_numeric_type_t      data_type;
	ccs_numeric_t           quantization;
	ccs_float_t             mu, sigma;
	ccs_interval_t          interval;

	err = ccs_create_normal_distribution(
		CCS_NUM_FLOAT,
		1.0,
		2.0,
		CCS_LINEAR,
		CCSF(0.0),
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_object_get_type(distrib, &otype);
	assert( err == CCS_SUCCESS );
	assert( otype == CCS_DISTRIBUTION );

	err = ccs_distribution_get_type(distrib, &dtype);
	assert( err == CCS_SUCCESS );
	assert( dtype == CCS_NORMAL );

	err = ccs_distribution_get_data_type(distrib, &data_type);
	assert( err == CCS_SUCCESS );
	assert( data_type == CCS_NUM_FLOAT );

	err = ccs_distribution_get_scale_type(distrib, &stype);
	assert( err == CCS_SUCCESS );
	assert( stype == CCS_LINEAR );

	err = ccs_distribution_get_quantization(distrib, &quantization);
	assert( err == CCS_SUCCESS );
	assert( quantization.f == 0.0 );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_FLOAT );
	assert( interval.lower.f == -CCS_INFINITY );
	assert( interval.lower_included == CCS_FALSE );
	assert( interval.upper.f == CCS_INFINITY );
	assert( interval.upper_included == CCS_FALSE );

	err = ccs_normal_distribution_get_parameters(distrib, &mu, &sigma);
	assert( err == CCS_SUCCESS );
	assert( mu == 1.0 );
	assert( sigma == 2.0 );

	err = ccs_object_get_refcount(distrib, &refcount);
	assert( err == CCS_SUCCESS );
	assert( refcount == 1 );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
}

static void test_create_normal_distribution_errors() {
	ccs_distribution_t      distrib = NULL;
	ccs_error_t             err = CCS_SUCCESS;

	// check wrong data_type
	err = ccs_create_normal_distribution(
		(ccs_numeric_type_t)3,
		1.0,
		2.0,
		CCS_LINEAR,
		CCSF(0.0),
		&distrib);
	assert( err == -CCS_INVALID_TYPE );

	// check wrong data_type
	err = ccs_create_normal_distribution(
		CCS_NUM_FLOAT,
		1.0,
		2.0,
		(ccs_scale_type_t)0xdeadbeef,
		CCSF(0.0),
		&distrib);
	assert( err == -CCS_INVALID_SCALE );

	// check wrong quantization
	err = ccs_create_normal_distribution(
		CCS_NUM_FLOAT,
		1.0,
		2.0,
		CCS_LINEAR,
		CCSF(-1.0),
		&distrib);
	assert( err == -CCS_INVALID_VALUE );

	// check wrong pointer
	err = ccs_create_normal_distribution(
		CCS_NUM_FLOAT,
		1.0,
		2.0,
		CCS_LINEAR,
		CCSF(0.0),
		NULL);
	assert( err == -CCS_INVALID_VALUE );


}

static void to_float(ccs_numeric_t * values, size_t length) {
	for (size_t i = 0; i < length; i++) {
		values[i].f = values[i].i;
	}
}

static void to_log(ccs_numeric_t * values, size_t length) {
	for (size_t i = 0; i < length; i++) {
		values[i].f = log(values[i].f);
	}
}

static void test_normal_distribution_int() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 1;
	const ccs_float_t  sigma = 2;
	ccs_numeric_t      samples[num_samples];
	double             mean, sig;
	ccs_interval_t     interval;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_NUM_INTEGER,
		mu,
		sigma,
		CCS_LINEAR,
		CCSI(0),
		&distrib);
	assert( err == CCS_SUCCESS );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_INTEGER );
	assert( interval.lower.i == CCS_INT_MIN );
	assert( interval.lower_included == CCS_TRUE );
	assert( interval.upper.i == CCS_INT_MAX );
	assert( interval.upper_included == CCS_TRUE );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	to_float(samples, num_samples);
	mean = gsl_stats_mean((double*)samples, 1, num_samples);
	//fprintf(stderr, "mu: %lf \n", mean);
	assert( mean < mu + 0.1 );
	assert( mean > mu - 0.1 );
	sig    = gsl_stats_sd_m((double*)samples, 1, num_samples, mu);
	//fprintf(stderr, "sig: %lf \n", sig);
	assert( sig < sigma + 0.1 );
	assert( sig > sigma - 0.1 );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_normal_distribution_float() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 1;
	const ccs_float_t  sigma = 2;
	ccs_numeric_t      samples[num_samples];
	double             mean, sig;
	ccs_interval_t     interval;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_NUM_FLOAT,
		mu,
		sigma,
		CCS_LINEAR,
		CCSF(0.0),
		&distrib);
	assert( err == CCS_SUCCESS );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_FLOAT );
	assert( interval.lower.f == -CCS_INFINITY );
	assert( interval.lower_included == CCS_FALSE );
	assert( interval.upper.f == CCS_INFINITY );
	assert( interval.upper_included == CCS_FALSE );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	mean = gsl_stats_mean((double*)samples, 1, num_samples);
	//fprintf(stderr, "mu: %lf \n", mean);
	assert( mean < mu + 0.1 );
	assert( mean > mu - 0.1 );
	sig    = gsl_stats_sd_m((double*)samples, 1, num_samples, mu);
	//fprintf(stderr, "sig: %lf \n", sig);
	assert( sig < sigma + 0.1 );
	assert( sig > sigma - 0.1 );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_normal_distribution_int_log() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 1;
	const ccs_float_t  sigma = 2;
	ccs_numeric_t      samples[num_samples];
	double             mean, sig;
	double             tmean, tsigma, alpha, zee, pdfa;
	ccs_interval_t     interval;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_NUM_INTEGER,
		mu,
		sigma,
		CCS_LOGARITHMIC,
		CCSI(0),
		&distrib);
	assert( err == CCS_SUCCESS );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_INTEGER );
	assert( interval.lower.i == 1 );
	assert( interval.lower_included == CCS_TRUE );
	assert( interval.upper.i == CCS_INT_MAX );
	assert( interval.upper_included == CCS_TRUE );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	to_float(samples, num_samples);
	to_log(samples, num_samples);
	mean = gsl_stats_mean((double*)samples, 1, num_samples);
	//fprintf(stderr, "mean: %lf \n", mean);
	// cutoff at 0.0 to have exp(v) >= 1
	// see https://en.wikipedia.org/wiki/Truncated_normal_distribution
	alpha = (log(0.5) - mu)/sigma;
	zee = (1.0 - gsl_cdf_ugaussian_P(alpha));
	pdfa = gsl_ran_ugaussian_pdf(alpha);
	tmean = mu + pdfa * sigma / zee;
	//fprintf(stderr, "tmean: %lf \n", tmean);
	assert( mean < tmean + 0.1 );
	assert( mean > tmean - 0.1 );
	sig    = gsl_stats_sd_m((double*)samples, 1, num_samples, tmean);
	//fprintf(stderr, "sig: %lf \n", sig);
	tsigma = sqrt( sigma * sigma * ( 1.0 + alpha * pdfa / zee - ( pdfa * pdfa )/( zee * zee ) ) );
	//fprintf(stderr, "tsig: %lf \n", tsigma);
	assert( sig < tsigma + 0.1 );
	assert( sig > tsigma - 1.1 );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_normal_distribution_float_log() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 1;
	const ccs_float_t  sigma = 2;
	ccs_numeric_t      samples[num_samples];
	double             mean, sig;
	ccs_interval_t     interval;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_NUM_FLOAT,
		mu,
		sigma,
		CCS_LOGARITHMIC,
		CCSF(0.0),
		&distrib);
	assert( err == CCS_SUCCESS );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_FLOAT );
	assert( interval.lower.f == 0.0 );
	assert( interval.lower_included == CCS_FALSE );
	assert( interval.upper.f == CCS_INFINITY );
	assert( interval.upper_included == CCS_FALSE );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	to_log(samples, num_samples);
	mean = gsl_stats_mean((double*)samples, 1, num_samples);
	//fprintf(stderr, "mu: %lf \n", mean);
	assert( mean < mu + 0.1 );
	assert( mean > mu - 0.1 );
	sig    = gsl_stats_sd_m((double*)samples, 1, num_samples, mu);
	//fprintf(stderr, "sig: %lf \n", sig);
	assert( sig < sigma + 0.1 );
	assert( sig > sigma - 0.1 );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_normal_distribution_int_quantize() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 1;
	const ccs_float_t  sigma = 2;
	const ccs_int_t    q = 2L;
	ccs_numeric_t      samples[num_samples];
	double             mean, sig;
	ccs_interval_t     interval;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_NUM_INTEGER,
		mu,
		sigma,
		CCS_LINEAR,
		CCSI(q),
		&distrib);
	assert( err == CCS_SUCCESS );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_INTEGER );
	assert( interval.lower.i == (CCS_INT_MIN/q)*q );
	assert( interval.lower_included == CCS_TRUE );
	assert( interval.upper.i == (CCS_INT_MAX/q)*q );
	assert( interval.upper_included == CCS_TRUE );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	to_float(samples, num_samples);
	mean = gsl_stats_mean((double*)samples, 1, num_samples);
	//fprintf(stderr, "mu: %lf \n", mean);
	assert( mean < mu + 0.1 );
	assert( mean > mu - 0.1 );
	sig    = gsl_stats_sd_m((double*)samples, 1, num_samples, mu);
	//fprintf(stderr, "sig: %lf \n", sig);
	assert( sig < sigma + 0.1 );
	assert( sig > sigma - 0.1 );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_normal_distribution_float_quantize() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 1;
	const ccs_float_t  sigma = 2;
	ccs_numeric_t      samples[num_samples];
	double             mean, sig;
	ccs_interval_t     interval;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_NUM_FLOAT,
		mu,
		sigma,
		CCS_LINEAR,
		CCSF(0.2),
		&distrib);
	assert( err == CCS_SUCCESS );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_FLOAT );
	assert( interval.lower.f == -CCS_INFINITY );
	assert( interval.lower_included == CCS_FALSE );
	assert( interval.upper.f == CCS_INFINITY );
	assert( interval.upper_included == CCS_FALSE );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	mean = gsl_stats_mean((double*)samples, 1, num_samples);
	//fprintf(stderr, "mu: %lf \n", mean);
	assert( mean < mu + 0.1 );
	assert( mean > mu - 0.1 );
	sig    = gsl_stats_sd_m((double*)samples, 1, num_samples, mu);
	//fprintf(stderr, "sig: %lf \n", sig);
	assert( sig < sigma + 0.1 );
	assert( sig > sigma - 0.1 );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_normal_distribution_int_log_quantize() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 3;
	const ccs_float_t  sigma = 2;
	const ccs_int_t    quantize = 2L;
	ccs_numeric_t      samples[num_samples];
	double             mean, sig;
	double             tmean, tsigma, alpha, zee, pdfa;
	ccs_interval_t     interval;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_NUM_INTEGER,
		mu,
		sigma,
		CCS_LOGARITHMIC,
		CCSI(quantize),
		&distrib);
	assert( err == CCS_SUCCESS );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_INTEGER );
	assert( interval.lower.i == quantize );
	assert( interval.lower_included == CCS_TRUE );
	assert( interval.upper.i == (CCS_INT_MAX/quantize)*quantize );
	assert( interval.upper_included == CCS_TRUE );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	to_float(samples, num_samples);
	to_log(samples, num_samples);
	mean = gsl_stats_mean((double*)samples, 1, num_samples);
	//fprintf(stderr, "mean: %lf \n", mean);
	// cutoff at 0.0 to have exp(v) >= 1
	// see https://en.wikipedia.org/wiki/Truncated_normal_distribution
	alpha = (log(0.5*quantize) - mu)/sigma;
	zee = (1.0 - gsl_cdf_ugaussian_P(alpha));
	pdfa = gsl_ran_ugaussian_pdf(alpha);
	tmean = mu + pdfa * sigma / zee;
	//fprintf(stderr, "tmean: %lf \n", tmean);
	assert( mean < tmean + 0.1 );
	assert( mean > tmean - 0.1 );
	sig    = gsl_stats_sd_m((double*)samples, 1, num_samples, tmean);
	//fprintf(stderr, "sig: %lf \n", sig);
	tsigma = sqrt( sigma * sigma * ( 1.0 + alpha * pdfa / zee - ( pdfa * pdfa )/( zee * zee ) ) );
	//fprintf(stderr, "tsig: %lf \n", tsigma);
	assert( sig < tsigma + 0.1 );
	assert( sig > tsigma - 1.1 );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

static void test_normal_distribution_float_log_quantize() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 3;
	const ccs_float_t  sigma = 2;
	const ccs_float_t  quantization = 2.0;
	ccs_numeric_t      samples[num_samples];
	double             mean, sig;
	double             tmean, tsigma, alpha, zee, pdfa;
	ccs_interval_t     interval;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_NUM_FLOAT,
		mu,
		sigma,
		CCS_LOGARITHMIC,
		CCSF(quantization),
		&distrib);
	assert( err == CCS_SUCCESS );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_FLOAT );
	assert( interval.lower.f == quantization );
	assert( interval.lower_included == CCS_TRUE );
	assert( interval.upper.f == CCS_INFINITY );
	assert( interval.upper_included == CCS_FALSE );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	to_log(samples, num_samples);
	mean = gsl_stats_mean((double*)samples, 1, num_samples);
	//fprintf(stderr, "mu: %lf \n", mean);
	// cutoff at log(quantization/2.0) to have quantized(exp(v)) >= quantization
	// see https://en.wikipedia.org/wiki/Truncated_normal_distribution
	alpha = (log(quantization*0.5) - mu)/sigma;
	zee = (1.0 - gsl_cdf_ugaussian_P(alpha));
	pdfa = gsl_ran_ugaussian_pdf(alpha);
	tmean = mu + pdfa * sigma / zee;
	//fprintf(stderr, "tmean: %lf \n", tmean);
	assert( mean < tmean + 0.1 );
	assert( mean > tmean - 0.1 );
	sig    = gsl_stats_sd_m((double*)samples, 1, num_samples, tmean);
	//fprintf(stderr, "sig: %lf \n", sig);
	tsigma = sqrt( sigma * sigma * ( 1.0 + alpha * pdfa / zee - ( pdfa * pdfa )/( zee * zee ) ) );
	//fprintf(stderr, "tsig: %lf \n", tsigma);
	assert( sig < tsigma + 0.1 );
	assert( sig > tsigma - 0.1 );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

int main(int argc, char *argv[]) {
	ccs_init();
	test_create_normal_distribution();
	test_create_normal_distribution_errors();
	test_normal_distribution_int();
	test_normal_distribution_int_log();
	test_normal_distribution_int_quantize();
	test_normal_distribution_int_log_quantize();
	test_normal_distribution_float();
	test_normal_distribution_float_log();
	test_normal_distribution_float_quantize();
	test_normal_distribution_float_log_quantize();
	return 0;
}
