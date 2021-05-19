#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>

void test_create_multivariate_distribution() {
	const size_t            num_distribs = 2;
	ccs_distribution_t      distrib = NULL;
	ccs_distribution_t      distribs[num_distribs];
	ccs_distribution_t      distribs_ret[num_distribs];
	ccs_result_t            err = CCS_SUCCESS;
	int32_t                 refcount;
	ccs_object_type_t       otype;
	ccs_distribution_type_t dtype;
	ccs_numeric_type_t      data_types[2];
	ccs_interval_t          intervals[num_distribs];
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
	ccs_distribution_t distrib = NULL, distribs[2];
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_distribs = 2;
	const size_t       num_samples = 10000;
	ccs_numeric_t      samples[num_samples*num_distribs];

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
		assert( samples[2*i].f >= -5.0 );
		assert( samples[2*i].f <   5.0 );
		assert( samples[2*i + 1].i >= -5 );
		assert( samples[2*i + 1].i <   5 );
	}
	double mean = gsl_stats_mean((double*)samples, 2, num_samples);
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
	ccs_distribution_t distrib = NULL, distribs[2];
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_distribs = 2;
	const size_t       num_samples = 10000;
	ccs_numeric_t      samples[num_samples*(num_distribs+1)];

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

	err = ccs_distribution_strided_samples(distrib, rng, num_samples, 3, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
		assert( samples[3*i].f >= -5.0 );
		assert( samples[3*i].f <   5.0 );
		assert( samples[3*i + 1].i >= -5 );
		assert( samples[3*i + 1].i <   5 );
	}
	double mean = gsl_stats_mean((double*)samples, 3, num_samples);
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
	ccs_distribution_t distrib = NULL, distribs[2];
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_distribs = 2;
	const size_t       num_samples = 10000;
	ccs_numeric_t      samples1[num_samples];
	ccs_numeric_t      samples2[num_samples];
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
	const size_t         num_distribs = 2;
	ccs_distribution_t   distrib = NULL, distribs[num_distribs];
	ccs_hyperparameter_t params[num_distribs];
	ccs_rng_t            rng = NULL;
	ccs_result_t         err = CCS_SUCCESS;
	const size_t         num_samples = 10000;
	ccs_datum_t          samples[num_samples*num_distribs];

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
		assert( samples[2*i].type == CCS_FLOAT );
		assert( samples[2*i].value.f >= -5.0 );
		assert( samples[2*i].value.f <   5.0 );
		assert( samples[2*i + 1].type == CCS_FLOAT );
		assert( samples[2*i + 1].value.f >= -4.0 );
		assert( samples[2*i + 1].value.f <   6.0 );
	}
	double mean = gsl_stats_mean((double*)samples, 4, num_samples);
	assert( mean < 0.0 + 0.1 );
	assert( mean > 0.0 - 0.1 );

	mean = gsl_stats_mean((double*)samples + 2, 4, num_samples);
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
	const size_t         num_distribs = 2;
	ccs_distribution_t   distrib = NULL, distribs[num_distribs];
	ccs_hyperparameter_t params[num_distribs];
	ccs_rng_t            rng = NULL;
	ccs_result_t         err = CCS_SUCCESS;
	const size_t         num_samples = 10000;
	ccs_datum_t          samples[num_samples*num_distribs];

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
		assert( samples[2*i].type == CCS_FLOAT );
		assert( samples[2*i].value.f >= -5.0 );
		assert( samples[2*i].value.f <   5.0 );
		assert( samples[2*i + 1].type == CCS_FLOAT );
		assert( samples[2*i + 1].value.f >= -4.0 );
		assert( samples[2*i + 1].value.f <   6.0 );
	}
	double mean = gsl_stats_mean((double*)samples, 4, num_samples);
	assert( mean < 0.0 + 0.1 );
	assert( mean > 0.0 - 0.1 );

	mean = gsl_stats_mean((double*)samples + 2, 4, num_samples);
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
