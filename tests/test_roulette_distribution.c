#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>

void test_create_roulette_distribution() {
	ccs_distribution_t      distrib = NULL;
	ccs_result_t            err = CCS_SUCCESS;
	int32_t                 refcount;
	ccs_object_type_t       otype;
	ccs_distribution_type_t dtype;
	ccs_numeric_type_t      data_type;
	ccs_interval_t          interval;
	const size_t            num_areas = 4;
	ccs_float_t             areas[num_areas];
	size_t                  num_areas_ret;
	ccs_float_t             areas_ret[num_areas];
	const ccs_float_t       epsilon = 1e-15;

	for(size_t i = 0; i < num_areas; i++) {
		areas[i] = (double)(i+1);
	}

	err = ccs_create_roulette_distribution(
		num_areas,
		areas,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_object_get_type(distrib, &otype);
	assert( err == CCS_SUCCESS );
	assert( otype == CCS_DISTRIBUTION );

	err = ccs_distribution_get_type(distrib, &dtype);
	assert( err == CCS_SUCCESS );
	assert( dtype == CCS_ROULETTE );

	err = ccs_distribution_get_data_type(distrib, &data_type);
	assert( err == CCS_SUCCESS );
	assert( data_type == CCS_NUM_INTEGER );

        err = ccs_distribution_get_bounds(distrib, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_INTEGER );
	assert( interval.lower.i == 0 );
	assert( interval.lower_included == CCS_TRUE );
	assert( interval.upper.i == 4 );
	assert( interval.upper_included == CCS_FALSE );

	err = ccs_roulette_distribution_get_num_areas(distrib, &num_areas_ret);
	assert( err == CCS_SUCCESS );
	assert( num_areas_ret == num_areas );

	err = ccs_roulette_distribution_get_areas(distrib, num_areas, areas_ret, &num_areas_ret);
	assert( err == CCS_SUCCESS );
	assert( num_areas_ret == num_areas );

	ccs_float_t inv_sum = 2.0 / (num_areas * (num_areas + 1));
	for (size_t i = 0; i < num_areas; i++) {
		assert( areas_ret[i] <= areas[i] * inv_sum + epsilon &&
		        areas_ret[i] >= areas[i] * inv_sum - epsilon );
	}

	err = ccs_object_get_refcount(distrib, &refcount);
	assert( err == CCS_SUCCESS );
	assert( refcount == 1 );

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
}

void test_create_roulette_distribution_errors() {
	ccs_distribution_t      distrib = NULL;
	ccs_result_t            err = CCS_SUCCESS;
	const size_t            num_areas = 4;
	ccs_float_t             areas[num_areas];

	for(size_t i = 0; i < num_areas; i++) {
		areas[i] = (double)(i+1);
	}

	err = ccs_create_roulette_distribution(
		0,
		areas,
		&distrib);
	assert( err == -CCS_INVALID_VALUE );

	err = ccs_create_roulette_distribution(
		SIZE_MAX,
		areas,
		&distrib);
	assert( err == -CCS_INVALID_VALUE );

	err = ccs_create_roulette_distribution(
		num_areas,
		NULL,
		&distrib);
	assert( err == -CCS_INVALID_VALUE );

	err = ccs_create_roulette_distribution(
		num_areas,
		areas,
		NULL);
	assert( err == -CCS_INVALID_VALUE );

	areas[1] = -2;
	err = ccs_create_roulette_distribution(
		num_areas,
		areas,
		&distrib);
	assert( err == -CCS_INVALID_VALUE );

}

void test_roulette_distribution() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	ccs_numeric_t      samples[num_samples];
	const size_t       num_areas = 4;
	ccs_float_t        areas[num_areas];
	int                counts[num_areas];

	for(size_t i = 0; i < num_areas; i++) {
		areas[i] = (double)(i+1);
	        counts[i] = 0;
	}

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_roulette_distribution(
		num_areas,
		areas,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	ccs_float_t sum = 0.0;
	ccs_float_t inv_sum = 0.0;
	for(size_t i = 0; i < num_areas; i++) {
		sum += areas[i];
	}
	inv_sum = 1.0 / sum;
	for(size_t i = 0; i < num_samples; i++) {
		assert( samples[i].i >=0 && samples[i].i < 4 );
		counts[samples[i].i]++;
	}
	for(size_t i = 0; i < num_areas; i++) {
		ccs_float_t target = num_samples * areas[i] * inv_sum;
		assert( counts[i] >= target * 0.95 && counts[i] <= target * 1.05 );
	}

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

void test_roulette_distribution_zero() {
	ccs_distribution_t distrib = NULL;
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_samples = 8000;
	ccs_numeric_t      samples[num_samples];
	const size_t       num_areas = 4;
	ccs_float_t        areas[num_areas];
	int                counts[num_areas];

	for(size_t i = 0; i < num_areas; i++) {
		areas[i] = (double)(i+1);
	        counts[i] = 0;
	}
	areas[1] = 0.0;

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_roulette_distribution(
		num_areas,
		areas,
		&distrib);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert( err == CCS_SUCCESS );

	ccs_float_t sum = 0.0;
	ccs_float_t inv_sum = 0.0;
	for(size_t i = 0; i < num_areas; i++) {
		sum += areas[i];
	}
	inv_sum = 1.0 / sum;
	for(size_t i = 0; i < num_samples; i++) {
		assert( samples[i].i >=0 && samples[i].i < 4 );
		counts[samples[i].i]++;
	}
	for(size_t i = 0; i < num_areas; i++) {
		ccs_float_t target = num_samples * areas[i] * inv_sum;
		assert( counts[i] >= target * 0.95 && counts[i] <= target * 1.05 );
	}

	err = ccs_release_object(distrib);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

void test_roulette_distribution_strided_sample() {
	ccs_distribution_t distrib1 = NULL;
	ccs_distribution_t distrib2 = NULL;
	ccs_rng_t          rng = NULL;
	ccs_result_t       err = CCS_SUCCESS;
	const size_t       num_samples = 10000;
	ccs_numeric_t      samples[num_samples*2];
	const size_t       num_areas = 4;
	ccs_float_t        areas1[num_areas];
	ccs_float_t        areas2[num_areas];
	int                counts1[num_areas];
	int                counts2[num_areas];

	for(size_t i = 0; i < num_areas; i++) {
		areas1[i] = (double)(i+1);
	        counts1[i] = 0;
		areas2[i] = (double)(i+2);
	        counts2[i] = 0;
	}

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_roulette_distribution(
		num_areas,
		areas1,
		&distrib1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_roulette_distribution(
		num_areas,
		areas2,
		&distrib2);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_strided_samples(distrib1, rng, num_samples, 2, samples);
	assert( err == CCS_SUCCESS );

	err = ccs_distribution_strided_samples(distrib2, rng, num_samples, 2, &(samples[0]) + 1);
	assert( err == CCS_SUCCESS );

	ccs_float_t sum = 0.0;
	ccs_float_t inv_sum = 0.0;
	for(size_t i = 0; i < num_areas; i++) {
		sum += areas1[i];
	}
	inv_sum = 1.0 / sum;
	for(size_t i = 0; i < num_samples; i++) {
		assert( samples[2*i].i >=0 && samples[2*i].i < 4 );
		counts1[samples[2*i].i]++;
	}
	for(size_t i = 0; i < num_areas; i++) {
		ccs_float_t target = num_samples * areas1[i] * inv_sum;
		assert( counts1[i] >= target * 0.95 && counts1[i] <= target * 1.05 );
	}

	sum = 0.0;
	inv_sum = 0.0;
	for(size_t i = 0; i < num_areas; i++) {
		sum += areas2[i];
	}
	inv_sum = 1.0 / sum;
	for(size_t i = 0; i < num_samples; i++) {
		assert( samples[2*i+1].i >=0 && samples[2*i+1].i < 4 );
		counts2[samples[2*i+1].i]++;
	}
	for(size_t i = 0; i < num_areas; i++) {
		ccs_float_t target = num_samples * areas2[i] * inv_sum;
		assert( counts2[i] >= target * 0.95 && counts2[i] <= target * 1.05 );
	}

	err = ccs_release_object(distrib1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(distrib2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

int main(int argc, char *argv[]) {
	ccs_init();
	test_create_roulette_distribution();
	test_create_roulette_distribution_errors();
	test_roulette_distribution();
	test_roulette_distribution_zero();
	test_roulette_distribution_strided_sample();
	return 0;
}
