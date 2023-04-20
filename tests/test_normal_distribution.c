#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>

#define NUM_SAMPLES 10000

static void
compare_distribution(
	ccs_distribution_t distrib,
	ccs_float_t        m,
	ccs_float_t        s,
	ccs_numeric_t      q)
{
	ccs_result_t            err = CCS_RESULT_SUCCESS;
	int32_t                 refcount;
	ccs_object_type_t       otype;
	ccs_distribution_type_t dtype;
	ccs_scale_type_t        stype;
	ccs_numeric_type_t      data_type;
	ccs_numeric_t           quantization;
	ccs_float_t             mu, sigma;
	ccs_interval_t          interval;

	err = ccs_object_get_type(distrib, &otype);
	assert(err == CCS_RESULT_SUCCESS);
	assert(otype == CCS_OBJECT_TYPE_DISTRIBUTION);

	err = ccs_distribution_get_type(distrib, &dtype);
	assert(err == CCS_RESULT_SUCCESS);
	assert(dtype == CCS_DISTRIBUTION_TYPE_NORMAL);

	err = ccs_distribution_get_data_types(distrib, &data_type);
	assert(err == CCS_RESULT_SUCCESS);
	assert(data_type == CCS_NUMERIC_TYPE_FLOAT);

	err = ccs_distribution_get_bounds(distrib, &interval);
	assert(err == CCS_RESULT_SUCCESS);
	assert(interval.type == CCS_NUMERIC_TYPE_FLOAT);
	assert(interval.lower.f == -CCS_INFINITY);
	assert(interval.lower_included == CCS_FALSE);
	assert(interval.upper.f == CCS_INFINITY);
	assert(interval.upper_included == CCS_FALSE);

	err = ccs_normal_distribution_get_properties(
		distrib, &mu, &sigma, &stype, &quantization);
	assert(err == CCS_RESULT_SUCCESS);
	assert(mu == m);
	assert(sigma == s);
	assert(stype == CCS_SCALE_TYPE_LINEAR);
	assert(quantization.f == q.f);

	err = ccs_object_get_refcount(distrib, &refcount);
	assert(err == CCS_RESULT_SUCCESS);
	assert(refcount == 1);
}

static void
test_create_normal_distribution()
{
	ccs_distribution_t distrib = NULL;
	ccs_result_t       err     = CCS_RESULT_SUCCESS;
	char              *buff;
	size_t             buff_size;

	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, 1.0, 2.0, CCS_SCALE_TYPE_LINEAR,
		CCSF(0.0), &distrib);
	assert(err == CCS_RESULT_SUCCESS);

	compare_distribution(distrib, 1.0, 2.0, CCSF(0.0));

	err = ccs_object_serialize(
		distrib, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		distrib, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&distrib, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	free(buff);

	compare_distribution(distrib, 1.0, 2.0, CCSF(0.0));

	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_create_normal_distribution_errors()
{
	ccs_distribution_t distrib = NULL;
	ccs_result_t       err     = CCS_RESULT_SUCCESS;

	// check wrong data_type
	err                        = ccs_create_normal_distribution(
                (ccs_numeric_type_t)CCS_DATA_TYPE_STRING, 1.0, 2.0,
                CCS_SCALE_TYPE_LINEAR, CCSF(0.0), &distrib);
	assert(err == CCS_RESULT_ERROR_INVALID_TYPE);

	// check wrong data_type
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, 1.0, 2.0, (ccs_scale_type_t)0xdeadbeef,
		CCSF(0.0), &distrib);
	assert(err == CCS_RESULT_ERROR_INVALID_SCALE);

	// check wrong quantization
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, 1.0, 2.0, CCS_SCALE_TYPE_LINEAR,
		CCSF(-1.0), &distrib);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);

	// check wrong pointer
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, 1.0, 2.0, CCS_SCALE_TYPE_LINEAR,
		CCSF(0.0), NULL);
	assert(err == CCS_RESULT_ERROR_INVALID_VALUE);
}

static void
to_float(ccs_numeric_t *values, size_t length)
{
	for (size_t i = 0; i < length; i++) {
		values[i].f = values[i].i;
	}
}

static void
to_log(ccs_numeric_t *values, size_t length)
{
	for (size_t i = 0; i < length; i++) {
		values[i].f = log(values[i].f);
	}
}

static void
test_normal_distribution_int()
{
	ccs_distribution_t distrib     = NULL;
	ccs_rng_t          rng         = NULL;
	ccs_result_t       err         = CCS_RESULT_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	const ccs_float_t  mu          = 1;
	const ccs_float_t  sigma       = 2;
	ccs_numeric_t      samples[NUM_SAMPLES];
	double             mean, sig;
	ccs_interval_t     interval;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_INT, mu, sigma, CCS_SCALE_TYPE_LINEAR, CCSI(0),
		&distrib);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_get_bounds(distrib, &interval);
	assert(err == CCS_RESULT_SUCCESS);
	assert(interval.type == CCS_NUMERIC_TYPE_INT);
	assert(interval.lower.i == CCS_INT_MIN);
	assert(interval.lower_included == CCS_TRUE);
	assert(interval.upper.i == CCS_INT_MAX);
	assert(interval.upper_included == CCS_TRUE);

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert(err == CCS_RESULT_SUCCESS);

	to_float(samples, num_samples);
	mean = gsl_stats_mean((double *)samples, 1, num_samples);
	assert(mean < mu + 0.1);
	assert(mean > mu - 0.1);
	sig = gsl_stats_sd_m((double *)samples, 1, num_samples, mu);
	assert(sig < sigma + 0.1);
	assert(sig > sigma - 0.1);

	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_normal_distribution_float()
{
	ccs_distribution_t distrib     = NULL;
	ccs_rng_t          rng         = NULL;
	ccs_result_t       err         = CCS_RESULT_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	const ccs_float_t  mu          = 1;
	const ccs_float_t  sigma       = 2;
	ccs_numeric_t      samples[NUM_SAMPLES];
	double             mean, sig;
	ccs_interval_t     interval;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, mu, sigma, CCS_SCALE_TYPE_LINEAR,
		CCSF(0.0), &distrib);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_get_bounds(distrib, &interval);
	assert(err == CCS_RESULT_SUCCESS);
	assert(interval.type == CCS_NUMERIC_TYPE_FLOAT);
	assert(interval.lower.f == -CCS_INFINITY);
	assert(interval.lower_included == CCS_FALSE);
	assert(interval.upper.f == CCS_INFINITY);
	assert(interval.upper_included == CCS_FALSE);

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert(err == CCS_RESULT_SUCCESS);

	mean = gsl_stats_mean((double *)samples, 1, num_samples);
	assert(mean < mu + 0.1);
	assert(mean > mu - 0.1);
	sig = gsl_stats_sd_m((double *)samples, 1, num_samples, mu);
	assert(sig < sigma + 0.1);
	assert(sig > sigma - 0.1);

	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_normal_distribution_int_log()
{
	ccs_distribution_t distrib     = NULL;
	ccs_rng_t          rng         = NULL;
	ccs_result_t       err         = CCS_RESULT_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	const ccs_float_t  mu          = 1;
	const ccs_float_t  sigma       = 2;
	ccs_numeric_t      samples[NUM_SAMPLES];
	double             mean, sig;
	double             tmean, tsigma, alpha, zee, pdfa;
	ccs_interval_t     interval;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_INT, mu, sigma, CCS_SCALE_TYPE_LOGARITHMIC,
		CCSI(0), &distrib);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_get_bounds(distrib, &interval);
	assert(err == CCS_RESULT_SUCCESS);
	assert(interval.type == CCS_NUMERIC_TYPE_INT);
	assert(interval.lower.i == 1);
	assert(interval.lower_included == CCS_TRUE);
	assert(interval.upper.i == CCS_INT_MAX);
	assert(interval.upper_included == CCS_TRUE);

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert(err == CCS_RESULT_SUCCESS);

	to_float(samples, num_samples);
	to_log(samples, num_samples);
	mean  = gsl_stats_mean((double *)samples, 1, num_samples);
	// cutoff at 0.0 to have exp(v) >= 1
	// see https://en.wikipedia.org/wiki/Truncated_normal_distribution
	alpha = (log(0.5) - mu) / sigma;
	zee   = (1.0 - gsl_cdf_ugaussian_P(alpha));
	pdfa  = gsl_ran_ugaussian_pdf(alpha);
	tmean = mu + pdfa * sigma / zee;
	assert(mean < tmean + 0.1);
	assert(mean > tmean - 0.1);
	sig = gsl_stats_sd_m((double *)samples, 1, num_samples, tmean);
	tsigma =
		sqrt(sigma * sigma *
		     (1.0 + alpha * pdfa / zee - (pdfa * pdfa) / (zee * zee)));
	assert(sig < tsigma + 0.1);
	assert(sig > tsigma - 1.1);

	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_normal_distribution_float_log()
{
	ccs_distribution_t distrib     = NULL;
	ccs_rng_t          rng         = NULL;
	ccs_result_t       err         = CCS_RESULT_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	const ccs_float_t  mu          = 1;
	const ccs_float_t  sigma       = 2;
	ccs_numeric_t      samples[NUM_SAMPLES];
	double             mean, sig;
	ccs_interval_t     interval;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, mu, sigma, CCS_SCALE_TYPE_LOGARITHMIC,
		CCSF(0.0), &distrib);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_get_bounds(distrib, &interval);
	assert(err == CCS_RESULT_SUCCESS);
	assert(interval.type == CCS_NUMERIC_TYPE_FLOAT);
	assert(interval.lower.f == 0.0);
	assert(interval.lower_included == CCS_FALSE);
	assert(interval.upper.f == CCS_INFINITY);
	assert(interval.upper_included == CCS_FALSE);

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert(err == CCS_RESULT_SUCCESS);

	to_log(samples, num_samples);
	mean = gsl_stats_mean((double *)samples, 1, num_samples);
	assert(mean < mu + 0.1);
	assert(mean > mu - 0.1);
	sig = gsl_stats_sd_m((double *)samples, 1, num_samples, mu);
	assert(sig < sigma + 0.1);
	assert(sig > sigma - 0.1);

	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_normal_distribution_int_quantize()
{
	ccs_distribution_t distrib     = NULL;
	ccs_rng_t          rng         = NULL;
	ccs_result_t       err         = CCS_RESULT_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	const ccs_float_t  mu          = 1;
	const ccs_float_t  sigma       = 2;
	const ccs_int_t    q           = 2L;
	ccs_numeric_t      samples[NUM_SAMPLES];
	double             mean, sig;
	ccs_interval_t     interval;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_INT, mu, sigma, CCS_SCALE_TYPE_LINEAR, CCSI(q),
		&distrib);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_get_bounds(distrib, &interval);
	assert(err == CCS_RESULT_SUCCESS);
	assert(interval.type == CCS_NUMERIC_TYPE_INT);
	assert(interval.lower.i == (CCS_INT_MIN / q) * q);
	assert(interval.lower_included == CCS_TRUE);
	assert(interval.upper.i == (CCS_INT_MAX / q) * q);
	assert(interval.upper_included == CCS_TRUE);

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert(err == CCS_RESULT_SUCCESS);

	to_float(samples, num_samples);
	mean = gsl_stats_mean((double *)samples, 1, num_samples);
	assert(mean < mu + 0.1);
	assert(mean > mu - 0.1);
	sig = gsl_stats_sd_m((double *)samples, 1, num_samples, mu);
	assert(sig < sigma + 0.1);
	assert(sig > sigma - 0.1);

	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_normal_distribution_float_quantize()
{
	ccs_distribution_t distrib     = NULL;
	ccs_rng_t          rng         = NULL;
	ccs_result_t       err         = CCS_RESULT_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	const ccs_float_t  mu          = 1;
	const ccs_float_t  sigma       = 2;
	ccs_numeric_t      samples[NUM_SAMPLES];
	double             mean, sig;
	ccs_interval_t     interval;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, mu, sigma, CCS_SCALE_TYPE_LINEAR,
		CCSF(0.2), &distrib);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_get_bounds(distrib, &interval);
	assert(err == CCS_RESULT_SUCCESS);
	assert(interval.type == CCS_NUMERIC_TYPE_FLOAT);
	assert(interval.lower.f == -CCS_INFINITY);
	assert(interval.lower_included == CCS_FALSE);
	assert(interval.upper.f == CCS_INFINITY);
	assert(interval.upper_included == CCS_FALSE);

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert(err == CCS_RESULT_SUCCESS);

	mean = gsl_stats_mean((double *)samples, 1, num_samples);
	assert(mean < mu + 0.1);
	assert(mean > mu - 0.1);
	sig = gsl_stats_sd_m((double *)samples, 1, num_samples, mu);
	assert(sig < sigma + 0.1);
	assert(sig > sigma - 0.1);

	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_normal_distribution_int_log_quantize()
{
	ccs_distribution_t distrib     = NULL;
	ccs_rng_t          rng         = NULL;
	ccs_result_t       err         = CCS_RESULT_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	const ccs_float_t  mu          = 3;
	const ccs_float_t  sigma       = 2;
	const ccs_int_t    quantize    = 2L;
	ccs_numeric_t      samples[NUM_SAMPLES];
	double             mean, sig;
	double             tmean, tsigma, alpha, zee, pdfa;
	ccs_interval_t     interval;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_INT, mu, sigma, CCS_SCALE_TYPE_LOGARITHMIC,
		CCSI(quantize), &distrib);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_get_bounds(distrib, &interval);
	assert(err == CCS_RESULT_SUCCESS);
	assert(interval.type == CCS_NUMERIC_TYPE_INT);
	assert(interval.lower.i == quantize);
	assert(interval.lower_included == CCS_TRUE);
	assert(interval.upper.i == (CCS_INT_MAX / quantize) * quantize);
	assert(interval.upper_included == CCS_TRUE);

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert(err == CCS_RESULT_SUCCESS);

	to_float(samples, num_samples);
	to_log(samples, num_samples);
	mean  = gsl_stats_mean((double *)samples, 1, num_samples);
	// cutoff at 0.0 to have exp(v) >= 1
	// see https://en.wikipedia.org/wiki/Truncated_normal_distribution
	alpha = (log(0.5 * quantize) - mu) / sigma;
	zee   = (1.0 - gsl_cdf_ugaussian_P(alpha));
	pdfa  = gsl_ran_ugaussian_pdf(alpha);
	tmean = mu + pdfa * sigma / zee;
	assert(mean < tmean + 0.1);
	assert(mean > tmean - 0.1);
	sig = gsl_stats_sd_m((double *)samples, 1, num_samples, tmean);
	tsigma =
		sqrt(sigma * sigma *
		     (1.0 + alpha * pdfa / zee - (pdfa * pdfa) / (zee * zee)));
	assert(sig < tsigma + 0.1);
	assert(sig > tsigma - 1.1);

	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_normal_distribution_float_log_quantize()
{
	ccs_distribution_t distrib      = NULL;
	ccs_rng_t          rng          = NULL;
	ccs_result_t       err          = CCS_RESULT_SUCCESS;
	const size_t       num_samples  = NUM_SAMPLES;
	const ccs_float_t  mu           = 3;
	const ccs_float_t  sigma        = 2;
	const ccs_float_t  quantization = 2.0;
	ccs_numeric_t      samples[NUM_SAMPLES];
	double             mean, sig;
	double             tmean, tsigma, alpha, zee, pdfa;
	ccs_interval_t     interval;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, mu, sigma, CCS_SCALE_TYPE_LOGARITHMIC,
		CCSF(quantization), &distrib);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_get_bounds(distrib, &interval);
	assert(err == CCS_RESULT_SUCCESS);
	assert(interval.type == CCS_NUMERIC_TYPE_FLOAT);
	assert(interval.lower.f == quantization);
	assert(interval.lower_included == CCS_TRUE);
	assert(interval.upper.f == CCS_INFINITY);
	assert(interval.upper_included == CCS_FALSE);

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert(err == CCS_RESULT_SUCCESS);

	to_log(samples, num_samples);
	mean  = gsl_stats_mean((double *)samples, 1, num_samples);
	// cutoff at log(quantization/2.0) to have quantized(exp(v)) >= quantization
	// see https://en.wikipedia.org/wiki/Truncated_normal_distribution
	alpha = (log(quantization * 0.5) - mu) / sigma;
	zee   = (1.0 - gsl_cdf_ugaussian_P(alpha));
	pdfa  = gsl_ran_ugaussian_pdf(alpha);
	tmean = mu + pdfa * sigma / zee;
	assert(mean < tmean + 0.1);
	assert(mean > tmean - 0.1);
	sig = gsl_stats_sd_m((double *)samples, 1, num_samples, tmean);
	tsigma =
		sqrt(sigma * sigma *
		     (1.0 + alpha * pdfa / zee - (pdfa * pdfa) / (zee * zee)));
	assert(sig < tsigma + 0.1);
	assert(sig > tsigma - 0.1);

	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_normal_distribution_strided_samples()
{
	ccs_distribution_t distrib1    = NULL;
	ccs_distribution_t distrib2    = NULL;
	ccs_rng_t          rng         = NULL;
	ccs_result_t       err         = CCS_RESULT_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	const ccs_float_t  mu1         = 1;
	const ccs_float_t  sigma1      = 2;
	const ccs_float_t  mu2         = 0;
	const ccs_float_t  sigma2      = 2;
	ccs_numeric_t      samples[NUM_SAMPLES * 2];
	double             mean, sig;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, mu1, sigma1, CCS_SCALE_TYPE_LINEAR,
		CCSF(0.0), &distrib1);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, mu2, sigma2, CCS_SCALE_TYPE_LINEAR,
		CCSF(0.0), &distrib2);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_strided_samples(
		distrib1, rng, num_samples, 2, samples);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_distribution_strided_samples(
		distrib2, rng, num_samples, 2, &(samples[0]) + 1);
	assert(err == CCS_RESULT_SUCCESS);

	mean = gsl_stats_mean((double *)samples, 2, num_samples);
	assert(mean < mu1 + 0.1);
	assert(mean > mu1 - 0.1);
	sig = gsl_stats_sd_m((double *)samples, 2, num_samples, mu1);
	assert(sig < sigma1 + 0.1);
	assert(sig > sigma1 - 0.1);

	mean = gsl_stats_mean((double *)samples + 1, 2, num_samples);
	assert(mean < mu2 + 0.1);
	assert(mean > mu2 - 0.1);
	sig = gsl_stats_sd_m((double *)samples + 1, 2, num_samples, mu2);
	assert(sig < sigma2 + 0.1);
	assert(sig > sigma2 - 0.1);

	err = ccs_release_object(distrib1);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(distrib2);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

static void
test_normal_distribution_soa_samples()
{
	ccs_distribution_t distrib     = NULL;
	ccs_rng_t          rng         = NULL;
	ccs_result_t       err         = CCS_RESULT_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	const ccs_float_t  mu          = 1;
	const ccs_float_t  sigma       = 2;
	ccs_numeric_t      samples[NUM_SAMPLES];
	double             mean, sig;
	ccs_interval_t     interval;
	ccs_numeric_t     *p_samples;

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, mu, sigma, CCS_SCALE_TYPE_LINEAR,
		CCSF(0.0), &distrib);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_distribution_get_bounds(distrib, &interval);
	assert(err == CCS_RESULT_SUCCESS);
	assert(interval.type == CCS_NUMERIC_TYPE_FLOAT);
	assert(interval.lower.f == -CCS_INFINITY);
	assert(interval.lower_included == CCS_FALSE);
	assert(interval.upper.f == CCS_INFINITY);
	assert(interval.upper_included == CCS_FALSE);

	p_samples = &(samples[0]);
	err       = ccs_distribution_soa_samples(
                distrib, rng, num_samples, &p_samples);
	assert(err == CCS_RESULT_SUCCESS);

	mean = gsl_stats_mean((double *)samples, 1, num_samples);
	assert(mean < mu + 0.1);
	assert(mean > mu - 0.1);
	sig = gsl_stats_sd_m((double *)samples, 1, num_samples, mu);
	assert(sig < sigma + 0.1);
	assert(sig > sigma - 0.1);

	err = ccs_release_object(distrib);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

int
main()
{
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
	test_normal_distribution_strided_samples();
	test_normal_distribution_soa_samples();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
