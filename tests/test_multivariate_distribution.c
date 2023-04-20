#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>

#define NUM_DISTRIBS 2
#define NUM_SAMPLES  10000

static void
compare_distribution(
	ccs_distribution_t distrib,
	size_t             num_distribs,
	ccs_distribution_t distribs[])
{
	ccs_error_t             err = CCS_SUCCESS;
	ccs_distribution_t      distribs_ret[NUM_DISTRIBS];
	int32_t                 refcount;
	ccs_object_type_t       otype;
	ccs_distribution_type_t dtype;
	ccs_numeric_type_t      data_types[NUM_DISTRIBS];
	ccs_interval_t          intervals[NUM_DISTRIBS];
	size_t                  num_distribs_ret;

	err = ccs_object_get_type(distrib, &otype);
	assert(err == CCS_SUCCESS);
	assert(otype == CCS_OBJECT_TYPE_DISTRIBUTION);

	err = ccs_distribution_get_type(distrib, &dtype);
	assert(err == CCS_SUCCESS);
	assert(dtype == CCS_DISTRIBUTION_TYPE_MULTIVARIATE);

	err = ccs_distribution_get_data_types(distrib, data_types);
	assert(err == CCS_SUCCESS);
	assert(data_types[0] == CCS_NUMERIC_TYPE_FLOAT);
	assert(data_types[1] == CCS_NUMERIC_TYPE_INT);

	err = ccs_distribution_get_bounds(distrib, intervals);
	assert(err == CCS_SUCCESS);
	assert(intervals[0].type == CCS_NUMERIC_TYPE_FLOAT);
	assert(intervals[0].lower.f == -5.0);
	assert(intervals[0].lower_included == CCS_TRUE);
	assert(intervals[0].upper.f == 5.0);
	assert(intervals[0].upper_included == CCS_FALSE);
	assert(intervals[1].type == CCS_NUMERIC_TYPE_INT);
	assert(intervals[1].lower.i == -5);
	assert(intervals[1].lower_included == CCS_TRUE);
	assert(intervals[1].upper.i == 5);
	assert(intervals[1].upper_included == CCS_FALSE);

	err = ccs_multivariate_distribution_get_num_distributions(
		distrib, &num_distribs_ret);
	assert(err == CCS_SUCCESS);
	assert(num_distribs_ret == num_distribs);

	err = ccs_multivariate_distribution_get_distributions(
		distrib, num_distribs, distribs_ret, &num_distribs_ret);
	assert(err == CCS_SUCCESS);
	assert(num_distribs_ret == num_distribs);
	for (size_t i = 0; i < num_distribs; i++) {
		ccs_interval_t interval_ref;
		err = ccs_distribution_get_bounds(distribs[i], &interval_ref);
		assert(err == CCS_SUCCESS);

		assert(intervals[i].type == interval_ref.type);
		if (intervals[i].type == CCS_NUMERIC_TYPE_FLOAT) {
			assert(intervals[i].lower.f == interval_ref.lower.f);
			assert(intervals[i].lower_included ==
			       interval_ref.lower_included);
			assert(intervals[i].upper.f == interval_ref.upper.f);
			assert(intervals[i].upper_included ==
			       interval_ref.upper_included);
		} else {
			assert(intervals[i].lower.i == interval_ref.lower.i);
			assert(intervals[i].lower_included ==
			       interval_ref.lower_included);
			assert(intervals[i].upper.i == interval_ref.upper.i);
			assert(intervals[i].upper_included ==
			       interval_ref.upper_included);
		}
	}

	err = ccs_object_get_refcount(distrib, &refcount);
	assert(err == CCS_SUCCESS);
	assert(refcount == 1);
}

void
test_create_multivariate_distribution()
{
	const size_t       num_distribs = NUM_DISTRIBS;
	ccs_distribution_t distrib      = NULL;
	ccs_distribution_t distribs[NUM_DISTRIBS];
	ccs_error_t        err = CCS_SUCCESS;
	char              *buff;
	size_t             buff_size;

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_FLOAT, CCSF(-5.0), CCSF(5.0),
		CCS_SCALE_TYPE_LINEAR, CCSF(0.0), distribs);
	assert(err == CCS_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_INT, CCSI(-5), CCSI(5), CCS_SCALE_TYPE_LINEAR,
		CCSI(0), distribs + 1);
	assert(err == CCS_SUCCESS);

	err = ccs_create_multivariate_distribution(
		num_distribs, distribs, &distrib);
	assert(err == CCS_SUCCESS);

	compare_distribution(distrib, num_distribs, distribs);

	err = ccs_object_serialize(
		distrib, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		distrib, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);

	err = ccs_release_object(distrib);
	assert(err == CCS_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&distrib, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_SUCCESS);
	free(buff);

	compare_distribution(distrib, num_distribs, distribs);

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert(err == CCS_SUCCESS);
	}

	err = ccs_release_object(distrib);
	assert(err == CCS_SUCCESS);
}

void
test_multivariate_distribution()
{
	ccs_distribution_t distrib      = NULL, distribs[NUM_DISTRIBS];
	ccs_rng_t          rng          = NULL;
	ccs_error_t        err          = CCS_SUCCESS;
	const size_t       num_distribs = NUM_DISTRIBS;
	const size_t       num_samples  = NUM_SAMPLES;
	ccs_numeric_t      samples[NUM_SAMPLES * NUM_DISTRIBS];

	err = ccs_create_rng(&rng);
	assert(err == CCS_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_FLOAT, CCSF(-5.0), CCSF(5.0),
		CCS_SCALE_TYPE_LINEAR, CCSF(0.0), distribs);
	assert(err == CCS_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_INT, CCSI(-5), CCSI(5), CCS_SCALE_TYPE_LINEAR,
		CCSI(0), distribs + 1);
	assert(err == CCS_SUCCESS);

	err = ccs_create_multivariate_distribution(
		num_distribs, distribs, &distrib);
	assert(err == CCS_SUCCESS);

	err = ccs_distribution_samples(distrib, rng, num_samples, samples);
	assert(err == CCS_SUCCESS);

	for (size_t i = 0; i < num_samples; i++) {
		assert(samples[NUM_DISTRIBS * i].f >= -5.0);
		assert(samples[NUM_DISTRIBS * i].f < 5.0);
		assert(samples[NUM_DISTRIBS * i + 1].i >= -5);
		assert(samples[NUM_DISTRIBS * i + 1].i < 5);
	}
	double mean =
		gsl_stats_mean((double *)samples, NUM_DISTRIBS, num_samples);
	assert(mean < 0.0 + 0.1);
	assert(mean > 0.0 - 0.1);

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert(err == CCS_SUCCESS);
	}
	err = ccs_release_object(distrib);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_SUCCESS);
}

void
test_multivariate_distribution_strided_samples()
{
	ccs_distribution_t distrib      = NULL, distribs[NUM_DISTRIBS];
	ccs_rng_t          rng          = NULL;
	ccs_error_t        err          = CCS_SUCCESS;
	const size_t       num_distribs = NUM_DISTRIBS;
	const size_t       num_samples  = NUM_SAMPLES;
	ccs_numeric_t      samples[NUM_SAMPLES * (NUM_DISTRIBS + 1)];

	err = ccs_create_rng(&rng);
	assert(err == CCS_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_FLOAT, CCSF(-5.0), CCSF(5.0),
		CCS_SCALE_TYPE_LINEAR, CCSF(0.0), distribs);
	assert(err == CCS_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_INT, CCSI(-5), CCSI(5), CCS_SCALE_TYPE_LINEAR,
		CCSI(0), distribs + 1);
	assert(err == CCS_SUCCESS);

	err = ccs_create_multivariate_distribution(
		num_distribs, distribs, &distrib);
	assert(err == CCS_SUCCESS);

	err = ccs_distribution_strided_samples(
		distrib, rng, num_samples, NUM_DISTRIBS + 1, samples);
	assert(err == CCS_SUCCESS);

	for (size_t i = 0; i < num_samples; i++) {
		assert(samples[(NUM_DISTRIBS + 1) * i].f >= -5.0);
		assert(samples[(NUM_DISTRIBS + 1) * i].f < 5.0);
		assert(samples[(NUM_DISTRIBS + 1) * i + 1].i >= -5);
		assert(samples[(NUM_DISTRIBS + 1) * i + 1].i < 5);
	}
	double mean = gsl_stats_mean(
		(double *)samples, NUM_DISTRIBS + 1, num_samples);
	assert(mean < 0.0 + 0.1);
	assert(mean > 0.0 - 0.1);

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert(err == CCS_SUCCESS);
	}
	err = ccs_release_object(distrib);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_SUCCESS);
}

void
test_multivariate_distribution_soa_samples()
{
	ccs_distribution_t distrib      = NULL, distribs[NUM_DISTRIBS];
	ccs_rng_t          rng          = NULL;
	ccs_error_t        err          = CCS_SUCCESS;
	const size_t       num_distribs = NUM_DISTRIBS;
	const size_t       num_samples  = NUM_SAMPLES;
	ccs_numeric_t      samples1[NUM_SAMPLES];
	ccs_numeric_t      samples2[NUM_SAMPLES];
	ccs_numeric_t     *samples[] = {samples1, samples2};

	err                          = ccs_create_rng(&rng);
	assert(err == CCS_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_FLOAT, CCSF(-5.0), CCSF(5.0),
		CCS_SCALE_TYPE_LINEAR, CCSF(0.0), distribs);
	assert(err == CCS_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_INT, CCSI(-5), CCSI(5), CCS_SCALE_TYPE_LINEAR,
		CCSI(0), distribs + 1);
	assert(err == CCS_SUCCESS);

	err = ccs_create_multivariate_distribution(
		num_distribs, distribs, &distrib);
	assert(err == CCS_SUCCESS);

	err = ccs_distribution_soa_samples(distrib, rng, num_samples, samples);
	assert(err == CCS_SUCCESS);

	for (size_t i = 0; i < num_samples; i++) {
		assert(samples1[i].f >= -5.0);
		assert(samples1[i].f < 5.0);
		assert(samples2[i].i >= -5);
		assert(samples2[i].i < 5);
	}
	double mean = gsl_stats_mean((double *)samples1, 1, num_samples);
	assert(mean < 0.0 + 0.1);
	assert(mean > 0.0 - 0.1);

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert(err == CCS_SUCCESS);
	}
	err = ccs_release_object(distrib);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_SUCCESS);
}

void
test_distribution_parameters_sample()
{
	const size_t       num_distribs = NUM_DISTRIBS;
	ccs_distribution_t distrib      = NULL, distribs[NUM_DISTRIBS];
	ccs_parameter_t    params[NUM_DISTRIBS];
	ccs_rng_t          rng         = NULL;
	ccs_error_t        err         = CCS_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_datum_t        samples[NUM_SAMPLES * NUM_DISTRIBS];

	err = ccs_create_rng(&rng);
	assert(err == CCS_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_FLOAT, CCSF(-4.0), CCSF(4.0),
		CCS_SCALE_TYPE_LINEAR, CCSF(0.0), distribs);
	assert(err == CCS_SUCCESS);

	err = ccs_create_uniform_distribution(
		CCS_NUMERIC_TYPE_FLOAT, CCSF(-3.0), CCSF(5.0),
		CCS_SCALE_TYPE_LINEAR, CCSF(0.0), distribs + 1);
	assert(err == CCS_SUCCESS);

	err = ccs_create_numerical_parameter(
		"param1", CCS_NUMERIC_TYPE_FLOAT, CCSF(-5.0), CCSF(5.0),
		CCSF(0.0), CCSF(0.0), params);
	assert(err == CCS_SUCCESS);

	err = ccs_create_numerical_parameter(
		"param2", CCS_NUMERIC_TYPE_FLOAT, CCSF(-4.0), CCSF(6.0),
		CCSF(0.0), CCSF(1.0), params + 1);
	assert(err == CCS_SUCCESS);

	err = ccs_create_multivariate_distribution(
		num_distribs, distribs, &distrib);
	assert(err == CCS_SUCCESS);

	err = ccs_distribution_parameters_samples(
		distrib, rng, params, num_samples, samples);
	assert(err == CCS_SUCCESS);

	for (size_t i = 0; i < num_samples; i++) {
		assert(samples[NUM_DISTRIBS * i].type == CCS_DATA_TYPE_FLOAT);
		assert(samples[NUM_DISTRIBS * i].value.f >= -5.0);
		assert(samples[NUM_DISTRIBS * i].value.f < 5.0);
		assert(samples[NUM_DISTRIBS * i + 1].type ==
		       CCS_DATA_TYPE_FLOAT);
		assert(samples[NUM_DISTRIBS * i + 1].value.f >= -4.0);
		assert(samples[NUM_DISTRIBS * i + 1].value.f < 6.0);
	}
	double mean = gsl_stats_mean(
		(double *)samples, NUM_DISTRIBS * 2, num_samples);
	assert(mean < 0.0 + 0.1);
	assert(mean > 0.0 - 0.1);

	mean = gsl_stats_mean(
		(double *)samples + 2, NUM_DISTRIBS * 2, num_samples);
	assert(mean < 1.0 + 0.1);
	assert(mean > 1.0 - 0.1);

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert(err == CCS_SUCCESS);
		err = ccs_release_object(params[i]);
		assert(err == CCS_SUCCESS);
	}
	err = ccs_release_object(distrib);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_SUCCESS);
}

void
test_distribution_parameters_sample_oversampling()
{
	const size_t       num_distribs = NUM_DISTRIBS;
	ccs_distribution_t distrib      = NULL, distribs[NUM_DISTRIBS];
	ccs_parameter_t    params[NUM_DISTRIBS];
	ccs_rng_t          rng         = NULL;
	ccs_error_t        err         = CCS_SUCCESS;
	const size_t       num_samples = NUM_SAMPLES;
	ccs_datum_t        samples[NUM_SAMPLES * NUM_DISTRIBS];

	err = ccs_create_rng(&rng);
	assert(err == CCS_SUCCESS);

	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, 0.0, 4.0, CCS_SCALE_TYPE_LINEAR,
		CCSF(0.0), distribs);
	assert(err == CCS_SUCCESS);

	err = ccs_create_normal_distribution(
		CCS_NUMERIC_TYPE_FLOAT, 1.0, 4.0, CCS_SCALE_TYPE_LINEAR,
		CCSF(0.0), distribs + 1);
	assert(err == CCS_SUCCESS);

	err = ccs_create_numerical_parameter(
		"param1", CCS_NUMERIC_TYPE_FLOAT, CCSF(-5.0), CCSF(5.0),
		CCSF(0.0), CCSF(0.0), params);
	assert(err == CCS_SUCCESS);

	err = ccs_create_numerical_parameter(
		"param2", CCS_NUMERIC_TYPE_FLOAT, CCSF(-4.0), CCSF(6.0),
		CCSF(0.0), CCSF(1.0), params + 1);
	assert(err == CCS_SUCCESS);

	err = ccs_create_multivariate_distribution(
		num_distribs, distribs, &distrib);
	assert(err == CCS_SUCCESS);

	err = ccs_distribution_parameters_samples(
		distrib, rng, params, num_samples, samples);
	assert(err == CCS_SUCCESS);

	for (size_t i = 0; i < num_samples; i++) {
		assert(samples[NUM_DISTRIBS * i].type == CCS_DATA_TYPE_FLOAT);
		assert(samples[NUM_DISTRIBS * i].value.f >= -5.0);
		assert(samples[NUM_DISTRIBS * i].value.f < 5.0);
		assert(samples[NUM_DISTRIBS * i + 1].type ==
		       CCS_DATA_TYPE_FLOAT);
		assert(samples[NUM_DISTRIBS * i + 1].value.f >= -4.0);
		assert(samples[NUM_DISTRIBS * i + 1].value.f < 6.0);
	}
	double mean = gsl_stats_mean(
		(double *)samples, NUM_DISTRIBS * 2, num_samples);
	assert(mean < 0.0 + 0.1);
	assert(mean > 0.0 - 0.1);

	mean = gsl_stats_mean(
		(double *)samples + 2, NUM_DISTRIBS * 2, num_samples);
	assert(mean < 1.0 + 0.1);
	assert(mean > 1.0 - 0.1);

	for (size_t i = 0; i < num_distribs; i++) {
		err = ccs_release_object(distribs[i]);
		assert(err == CCS_SUCCESS);
		err = ccs_release_object(params[i]);
		assert(err == CCS_SUCCESS);
	}
	err = ccs_release_object(distrib);
	assert(err == CCS_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_SUCCESS);
}

int
main()
{
	ccs_init();
	test_create_multivariate_distribution();
	test_multivariate_distribution();
	test_multivariate_distribution_strided_samples();
	test_multivariate_distribution_soa_samples();
	test_distribution_parameters_sample();
	test_distribution_parameters_sample_oversampling();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
