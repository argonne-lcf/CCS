#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>

#define NUM_DISTRIBS 2
#define NUM_SAMPLES 10000

void test_create_multivariate_distribution() {
	const size_t            num_distribs = NUM_DISTRIBS;
	ccs_distribution_t      distrib = NULL;
	ccs_distribution_t      distribs[NUM_DISTRIBS];
	ccs_distribution_t      distribs_ret[NUM_DISTRIBS];
	ccs_result_t            err = CCS_SUCCESS;
	int32_t                 refcount;
	ccs_object_type_t       otype;
	ccs_distribution_type_t dtype;
	ccs_numeric_type_t      data_types[NUM_DISTRIBS];
	ccs_interval_t          intervals[NUM_DISTRIBS];
	size_t                  num_distribs_ret;

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(-5.0),
		CCSF(5.0),
		CCS_LINEAR,
		CCSF(0.0),
		distribs);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(-5),
		CCSI(5),
		CCS_LINEAR,
		CCSI(0),
		distribs + 1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_multivariate_distribution(
		num_distribs,
		distribs,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_object_get_type(distrib, &otype);
	assert( err == CCS_SUCCESS );
	assert( otype == CCS_DISTRIBUTION );

	err = ccs_distribution_get_type(distrib, &dtype);
	assert( err == CCS_SUCCESS );
	assert( dtype == CCS_MULTIVARIATE );

	err = ccs_distribution_get_data_types(distrib, data_types);
	assert( err == CCS_SUCCESS );
	assert( data_types[0] == CCS_NUM_FLOAT );
	assert( data_types[1] == CCS_NUM_INTEGER );

        err = ccs_distribution_get_bounds(distrib, intervals);
	assert( err == CCS_SUCCESS );
	assert( intervals[0].type == CCS_NUM_FLOAT );
	assert( intervals[0].lower.f == -5.0 );
	assert( intervals[0].lower_included == CCS_TRUE );
	assert( intervals[0].upper.f == 5.0 );
	assert( intervals[0].upper_included == CCS_FALSE );
	assert( intervals[1].type == CCS_NUM_INTEGER );
	assert( intervals[1].lower.i == -5 );
	assert( intervals[1].lower_included == CCS_TRUE );
	assert( intervals[1].upper.i == 5 );
	assert( intervals[1].upper_included == CCS_FALSE );

	err = ccs_multivariate_distribution_get_num_distributions(distrib, &num_distribs_ret);
	assert( err == CCS_SUCCESS );
	assert( num_distribs_ret == num_distribs );

	err = ccs_multivariate_distribution_get_distributions(distrib, num_distribs, distribs_ret, &num_distribs_ret);
	assert( err == CCS_SUCCESS );
	assert( num_distribs_ret == num_distribs );
	for (size_t i = 0; i < num_distribs; i++)
		assert( distribs_ret[i] == distribs[i] );

	err = ccs_object_get_refcount(distrib, &refcount);
	assert( err == CCS_SUCCESS );
	assert( refcount == 1 );

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
}

void test_multivariate_distribution() {
	ccs_distribution_t distrib = NULL, distribs[NUM_DISTRIBS];
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_distribs = NUM_DISTRIBS;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_numeric_t      samples[NUM_SAMPLES*NUM_DISTRIBS];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(-5.0),
		CCSF(5.0),
		CCS_LINEAR,
		CCSF(0.0),
		distribs);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(-5),
		CCSI(5),
		CCS_LINEAR,
		CCSI(0),
		distribs + 1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_multivariate_distribution(
		num_distribs,
		distribs,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
		assert( samples[NUM_DISTRIBS*i].f >= -5.0 );
		assert( samples[NUM_DISTRIBS*i].f <   5.0 );
		assert( samples[NUM_DISTRIBS*i + 1].i >= -5 );
		assert( samples[NUM_DISTRIBS*i + 1].i <   5 );
	}
	double mean = gsl_stats_mean((double*)samples, NUM_DISTRIBS, num_samples);
	assert( mean < 0.0 + 0.1 );
	assert( mean > 0.0 - 0.1 );

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

void test_multivariate_distribution_strided_samples() {
	ccs_distribution_t distrib = NULL, distribs[NUM_DISTRIBS];
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_distribs = NUM_DISTRIBS;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_numeric_t      samples[NUM_SAMPLES*(NUM_DISTRIBS+1)];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(-5.0),
		CCSF(5.0),
		CCS_LINEAR,
		CCSF(0.0),
		distribs);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(-5),
		CCSI(5),
		CCS_LINEAR,
		CCSI(0),
		distribs + 1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_multivariate_distribution(
		num_distribs,
		distribs,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_strided_samples(distrib, rng, num_samples, NUM_DISTRIBS+1, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
		assert( samples[(NUM_DISTRIBS+1)*i].f >= -5.0 );
		assert( samples[(NUM_DISTRIBS+1)*i].f <   5.0 );
		assert( samples[(NUM_DISTRIBS+1)*i + 1].i >= -5 );
		assert( samples[(NUM_DISTRIBS+1)*i + 1].i <   5 );
	}
	double mean = gsl_stats_mean((double*)samples, NUM_DISTRIBS+1, num_samples);
	assert( mean < 0.0 + 0.1 );
	assert( mean > 0.0 - 0.1 );

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

void test_multivariate_distribution_soa_samples() {
	ccs_distribution_t distrib = NULL, distribs[NUM_DISTRIBS];
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_distribs = NUM_DISTRIBS;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_numeric_t      samples1[NUM_SAMPLES];
	ccs_numeric_t      samples2[NUM_SAMPLES];
	ccs_numeric_t     *samples[] = { samples1, samples2 };

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(-5.0),
		CCSF(5.0),
		CCS_LINEAR,
		CCSF(0.0),
		distribs);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_INTEGER,
		CCSI(-5),
		CCSI(5),
		CCS_LINEAR,
		CCSI(0),
		distribs + 1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_multivariate_distribution(
		num_distribs,
		distribs,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_soa_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
		assert( samples1[i].f >= -5.0 );
		assert( samples1[i].f <   5.0 );
		assert( samples2[i].i >= -5 );
		assert( samples2[i].i <   5 );
	}
	double mean = gsl_stats_mean((double*)samples1, 1, num_samples);
	assert( mean < 0.0 + 0.1 );
	assert( mean > 0.0 - 0.1 );

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

void test_distribution_hyperparameters_sample() {
	const size_t         num_distribs = NUM_DISTRIBS;
	ccs_distribution_t   distrib = NULL, distribs[NUM_DISTRIBS];
	ccs_hyperparameter_t params[NUM_DISTRIBS];
	ccs_rng_t            rng = NULL;
	ccs_result_t         err = CCS_SUCCESS;
	const size_t         num_samples = NUM_SAMPLES;
	ccs_datum_t          samples[NUM_SAMPLES*NUM_DISTRIBS];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(-4.0),
		CCSF(4.0),
		CCS_LINEAR,
		CCSF(0.0),
		distribs);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(-3.0),
		CCSF(5.0),
		CCS_LINEAR,
		CCSF(0.0),
		distribs + 1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_numerical_hyperparameter("param1", CCS_NUM_FLOAT,
	                                          CCSF(-5.0), CCSF(5.0),
	                                          CCSF(0.0), CCSF(0.0),
	                                          NULL, params);
	assert( err == CCS_SUCCESS );

	err = ccs_create_numerical_hyperparameter("param2", CCS_NUM_FLOAT,
	                                          CCSF(-4.0), CCSF(6.0),
	                                          CCSF(0.0), CCSF(1.0),
	                                          NULL, params + 1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_multivariate_distribution(
		num_distribs,
		distribs,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_hyperparameters_samples(distrib, rng, params, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
		assert( samples[NUM_DISTRIBS*i].type == CCS_FLOAT );
		assert( samples[NUM_DISTRIBS*i].value.f >= -5.0 );
		assert( samples[NUM_DISTRIBS*i].value.f <   5.0 );
		assert( samples[NUM_DISTRIBS*i + 1].type == CCS_FLOAT );
		assert( samples[NUM_DISTRIBS*i + 1].value.f >= -4.0 );
		assert( samples[NUM_DISTRIBS*i + 1].value.f <   6.0 );
	}
	double mean = gsl_stats_mean((double*)samples, NUM_DISTRIBS*2, num_samples);
	assert( mean < 0.0 + 0.1 );
	assert( mean > 0.0 - 0.1 );

	mean = gsl_stats_mean((double*)samples + 2, NUM_DISTRIBS*2, num_samples);
	assert( mean < 1.0 + 0.1 );
	assert( mean > 1.0 - 0.1 );

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(params[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

void test_distribution_hyperparameters_sample_oversampling() {
	const size_t         num_distribs = NUM_DISTRIBS;
	ccs_distribution_t   distrib = NULL, distribs[NUM_DISTRIBS];
	ccs_hyperparameter_t params[NUM_DISTRIBS];
	ccs_rng_t            rng = NULL;
	ccs_result_t         err = CCS_SUCCESS;
	const size_t         num_samples = NUM_SAMPLES;
	ccs_datum_t          samples[NUM_SAMPLES*NUM_DISTRIBS];

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );

	err = ccs_create_normal_distribution(
		CCS_NUM_FLOAT,
		0.0,
		4.0,
		CCS_LINEAR,
		CCSF(0.0),
		distribs);
	assert( err == CCS_SUCCESS );

	err = ccs_create_normal_distribution(
		CCS_NUM_FLOAT,
		1.0,
		4.0,
		CCS_LINEAR,
		CCSF(0.0),
		distribs + 1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_numerical_hyperparameter("param1", CCS_NUM_FLOAT,
	                                          CCSF(-5.0), CCSF(5.0),
	                                          CCSF(0.0), CCSF(0.0),
	                                          NULL, params);
	assert( err == CCS_SUCCESS );

	err = ccs_create_numerical_hyperparameter("param2", CCS_NUM_FLOAT,
	                                          CCSF(-4.0), CCSF(6.0),
	                                          CCSF(0.0), CCSF(1.0),
	                                          NULL, params + 1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_multivariate_distribution(
		num_distribs,
		distribs,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_hyperparameters_samples(distrib, rng, params, num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
		assert( samples[NUM_DISTRIBS*i].type == CCS_FLOAT );
		assert( samples[NUM_DISTRIBS*i].value.f >= -5.0 );
		assert( samples[NUM_DISTRIBS*i].value.f <   5.0 );
		assert( samples[NUM_DISTRIBS*i + 1].type == CCS_FLOAT );
		assert( samples[NUM_DISTRIBS*i + 1].value.f >= -4.0 );
		assert( samples[NUM_DISTRIBS*i + 1].value.f <   6.0 );
	}
	double mean = gsl_stats_mean((double*)samples, NUM_DISTRIBS*2, num_samples);
	assert( mean < 0.0 + 0.1 );
	assert( mean > 0.0 - 0.1 );

	mean = gsl_stats_mean((double*)samples + 2, NUM_DISTRIBS*2, num_samples);
	assert( mean < 1.0 + 0.1 );
	assert( mean > 1.0 - 0.1 );

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(params[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

int main() {
	ccs_init();
	test_create_multivariate_distribution();
	test_multivariate_distribution();
	test_multivariate_distribution_strided_samples();
	test_multivariate_distribution_soa_samples();
	test_distribution_hyperparameters_sample();
	test_distribution_hyperparameters_sample_oversampling();
}
