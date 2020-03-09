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
	size_t                  num_parameters;
	ccs_object_type_t       otype;
	ccs_distribution_type_t dtype;
	ccs_scale_type_t        stype;
	ccs_data_type_t         data_type;
	ccs_datum_t             parameters[2], quantization, mu, sigma;

	err = ccs_create_normal_distribution(
		CCS_FLOAT,
		1.0,
		2.0,
		CCS_LINEAR,
		0.0,
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
	assert( data_type == CCS_FLOAT );

	err = ccs_distribution_get_scale_type(distrib, &stype);
	assert( err == CCS_SUCCESS );
	assert( stype == CCS_LINEAR );

	err = ccs_distribution_get_quantization(distrib, &quantization);
	assert( err == CCS_SUCCESS );
	assert( quantization.type == CCS_FLOAT );
	assert( quantization.value.f == 0.0 );

	err = ccs_distribution_get_num_parameters(distrib, &num_parameters);
	assert( err == CCS_SUCCESS );
	assert( num_parameters == 2 );

	err = ccs_distribution_get_parameters(distrib, 2, parameters, &num_parameters);
	assert( err == CCS_SUCCESS );
	assert( num_parameters == 2 );
	assert( parameters[0].type == CCS_FLOAT );
	assert( parameters[0].value.f == 1.0 );
	assert( parameters[1].type == CCS_FLOAT );
	assert( parameters[1].value.f == 2.0 );

	err = ccs_normal_distribution_get_parameters(distrib, &mu, &sigma);
	assert( err == CCS_SUCCESS );
	assert( mu.type == CCS_FLOAT );
	assert( mu.value.f == 1.0 );
	assert( sigma.type == CCS_FLOAT );
	assert( sigma.value.f == 2.0 );

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
		CCS_OBJECT,
		1.0,
		2.0,
		CCS_LINEAR,
		0.0,
		&distrib);
	assert( err == -CCS_INVALID_TYPE );

	// check wrong data_type
	err = ccs_create_normal_distribution(
		CCS_FLOAT,
		1.0,
		2.0,
		0xdeadbeef,
		0.0,
		&distrib);
	assert( err == -CCS_INVALID_SCALE );

	// check wrong quantization
	err = ccs_create_normal_distribution(
		CCS_FLOAT,
		1.0,
		2.0,
		CCS_LINEAR,
		-1.0,
		&distrib);
	assert( err == -CCS_INVALID_VALUE );

	// check wrong pointer
	err = ccs_create_normal_distribution(
		CCS_FLOAT,
		1.0,
		2.0,
		CCS_LINEAR,
		0.0,
		NULL);
	assert( err == -CCS_INVALID_VALUE );


}

static void to_float(ccs_value_t * values, size_t length) {
	for (size_t i = 0; i < length; i++) {
		values[i].f = values[i].i;
	}
}

static void to_log(ccs_value_t * values, size_t length) {
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
	ccs_value_t        samples[num_samples];
	double mean, sig;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_INTEGER,
		mu,
		sigma,
		CCS_LINEAR,
		0L,
		&distrib);
	assert( err == CCS_SUCCESS );

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
}

static void test_normal_distribution_float() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 1;
	const ccs_float_t  sigma = 2;
	ccs_value_t        samples[num_samples];
	double mean, sig;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_FLOAT,
		mu,
		sigma,
		CCS_LINEAR,
		0.0,
		&distrib);
	assert( err == CCS_SUCCESS );

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
}

static void test_normal_distribution_int_log() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 1;
	const ccs_float_t  sigma = 2;
	ccs_value_t        samples[num_samples];
	double mean, sig;
	double tmean, tsigma, alpha, zee, pdfa;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_INTEGER,
		mu,
		sigma,
		CCS_LOGARITHMIC,
		0L,
		&distrib);
	assert( err == CCS_SUCCESS );

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
}

static void test_normal_distribution_float_log() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 1;
	const ccs_float_t  sigma = 2;
	ccs_value_t        samples[num_samples];
	double mean, sig;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_FLOAT,
		mu,
		sigma,
		CCS_LOGARITHMIC,
		0.0,
		&distrib);
	assert( err == CCS_SUCCESS );

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
}

static void test_normal_distribution_int_quantize() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 1;
	const ccs_float_t  sigma = 2;
	ccs_value_t        samples[num_samples];
	double mean, sig;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_INTEGER,
		mu,
		sigma,
		CCS_LINEAR,
		2L,
		&distrib);
	assert( err == CCS_SUCCESS );

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
}

static void test_normal_distribution_float_quantize() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 1;
	const ccs_float_t  sigma = 2;
	ccs_value_t        samples[num_samples];
	double mean, sig;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_FLOAT,
		mu,
		sigma,
		CCS_LINEAR,
		0.2,
		&distrib);
	assert( err == CCS_SUCCESS );

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
}

static void test_normal_distribution_int_log_quantize() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 3;
	const ccs_float_t  sigma = 2;
	const ccs_int_t    quantize = 2;
	ccs_value_t        samples[num_samples];
	double mean, sig;
	double tmean, tsigma, alpha, zee, pdfa;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_INTEGER,
		mu,
		sigma,
		CCS_LOGARITHMIC,
		quantize,
		&distrib);
	assert( err == CCS_SUCCESS );

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
}

static void test_normal_distribution_float_log_quantize() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_error_t        err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	const ccs_float_t  mu = 3;
	const ccs_float_t  sigma = 2;
	const ccs_float_t  quantization = 2.0;
	ccs_value_t        samples[num_samples];
	double mean, sig;
	double tmean, tsigma, alpha, zee, pdfa;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_normal_distribution(
		CCS_FLOAT,
		mu,
		sigma,
		CCS_LOGARITHMIC,
		quantization,
		&distrib);
	assert( err == CCS_SUCCESS );

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
