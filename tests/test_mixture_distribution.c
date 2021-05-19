#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>

void test_create_mixture_distribution() {
	ccs_distribution_t      distrib = NULL, distribs[2], distribs_ret[2];
	ccs_result_t            err = CCS_SUCCESS;
	int32_t                 refcount;
	ccs_object_type_t       otype;
	ccs_distribution_type_t dtype;
	ccs_numeric_type_t      data_type;
	ccs_interval_t          interval;
	const size_t            num_distribs = 2;
	ccs_float_t             weights[num_distribs];
	size_t                  num_distribs_ret;
	ccs_float_t             weights_ret[num_distribs];
	const ccs_float_t       epsilon = 1e-15;

	for(size_t i = 0; i < num_distribs; i++) {
		weights[i] = (double)(i+1);
	}

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(-10.0),
		CCSF(0.0),
		CCS_LINEAR,
		CCSF(0.0),
		distribs);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(0.0),
		CCSF(10.0),
		CCS_LINEAR,
		CCSF(0.0),
		distribs + 1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_mixture_distribution(
		num_distribs,
		distribs,
		weights,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_object_get_type(distrib, &otype);
	assert( err == CCS_SUCCESS );
	assert( otype == CCS_DISTRIBUTION );

	err = ccs_distribution_get_type(distrib, &dtype);
	assert( err == CCS_SUCCESS );
	assert( dtype == CCS_MIXTURE );

	err = ccs_distribution_get_data_types(distrib, &data_type);
	assert( err == CCS_SUCCESS );
	assert( data_type == CCS_NUM_FLOAT );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_FLOAT );
	assert( interval.lower.f == -10.0 );
	assert( interval.lower_included == CCS_TRUE );
	assert( interval.upper.f == 10.0 );
	assert( interval.upper_included == CCS_FALSE );

	err = ccs_mixture_distribution_get_num_distributions(distrib, &num_distribs_ret);
	assert( err == CCS_SUCCESS );
	assert( num_distribs_ret == num_distribs );

	err = ccs_mixture_distribution_get_distributions(distrib, num_distribs, distribs_ret, &num_distribs_ret);
	assert( err == CCS_SUCCESS );
	assert( num_distribs_ret == num_distribs );
	for (size_t i = 0; i < num_distribs; i++)
		assert( distribs_ret[i] == distribs[i] );

	err = ccs_mixture_distribution_get_weights(distrib, num_distribs, weights_ret, &num_distribs_ret);
	assert( err == CCS_SUCCESS );
	assert( num_distribs_ret == num_distribs );

	ccs_float_t inv_sum = 2.0 / (num_distribs * (num_distribs + 1));
	for (size_t i = 0; i < num_distribs; i++)
		assert( weights_ret[i] <= weights[i] * inv_sum + epsilon &&
		        weights_ret[i] >= weights[i] * inv_sum - epsilon );

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

void test_mixture_distribution() {
	ccs_distribution_t distrib = NULL, distribs[2];
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_distribs = 2;
	ccs_float_t        weights[num_distribs];
	const size_t       num_samples = 10000;
	ccs_numeric_t      samples[num_samples];

	for(size_t i = 0; i < num_distribs; i++) {
		weights[i] = 1.0;
	}

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(-9.0),
		CCSF(1.0),
		CCS_LINEAR,
		CCSF(0.0),
		distribs);
	assert( err == CCS_SUCCESS );

	err = ccs_create_uniform_distribution(
		CCS_NUM_FLOAT,
		CCSF(1.0),
		CCSF(11.0),
		CCS_LINEAR,
		CCSF(0.0),
		distribs + 1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_mixture_distribution(
		num_distribs,
		distribs,
		weights,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	double mean = gsl_stats_mean((double*)samples, 1, num_samples);
	assert( mean < 1.0 + 0.1 );
	assert( mean > 1.0 - 0.1 );

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

void test_mixture_distribution_strided_samples() {
	ccs_distribution_t t_distrib = NULL, distrib = NULL, distribs[2];
	ccs_distribution_t t_distribs[2];
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_t_distribs = 2;
	const size_t       num_distribs = 2;
	const size_t       num_samples = 10000;
	ccs_numeric_t      samples[num_samples*(num_distribs+1)];
	ccs_float_t        weights[] = { 1.0, 1.0 };

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

	t_distribs[0] = distrib;
	t_distribs[1] = distrib;
	err = ccs_create_mixture_distribution(
		num_t_distribs,
		t_distribs,
		weights,
		&t_distrib);
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
	err = ccs_release_object(t_distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

void test_mixture_distribution_soa_samples() {
	ccs_distribution_t t_distrib = NULL, distrib = NULL, distribs[2];
	ccs_distribution_t t_distribs[2];
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_t_distribs = 2;
	const size_t       num_distribs = 2;
	const size_t       num_samples = 10000;
	ccs_numeric_t      samples1[num_samples];
	ccs_numeric_t      samples2[num_samples];
	ccs_numeric_t     *samples[] = { samples1, samples2 };
	ccs_float_t        weights[] = { 1.0, 1.0 };

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

	t_distribs[0] = distrib;
	t_distribs[1] = distrib;
	err = ccs_create_mixture_distribution(
		num_t_distribs,
		t_distribs,
		weights,
		&t_distrib);
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
	err = ccs_release_object(t_distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}
int main() {
	ccs_init();
	test_create_mixture_distribution();
	test_mixture_distribution();
	test_mixture_distribution_strided_samples();
	test_mixture_distribution_soa_samples();
}
