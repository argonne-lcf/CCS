#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

void test_create() {
	ccs_hyperparameter_t       hyperparameter;
	ccs_hyperparameter_type_t  type;
	ccs_datum_t                default_value;
	ccs_error_t                err;
	ccs_bool_t                 check;
	const char                *name;
	void *                     user_data;
	ccs_distribution_t         distribution;
	ccs_distribution_type_t    dist_type;
	ccs_interval_t             interval;
	const size_t               num_possible_values = 4;
	ccs_datum_t                possible_values[num_possible_values];
	const size_t               default_value_index = 2;

	for(size_t i = 0; i < num_possible_values; i++) {
		possible_values[i].type = CCS_INTEGER;
		possible_values[i].value.i = (i+1)*2;
	}

	err = ccs_create_ordinal_hyperparameter("my_param", num_possible_values,
	                                        possible_values, default_value_index,
	                                        (void *)0xdeadbeef,
	                                        &hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_hyperparameter_get_type(hyperparameter, &type);
	assert( err == CCS_SUCCESS );
	assert( type == CCS_ORDINAL );

	err = ccs_hyperparameter_get_default_value(hyperparameter, &default_value);
	assert( err == CCS_SUCCESS );
	assert( default_value.type == CCS_INTEGER );
	assert( default_value.value.i == possible_values[default_value_index].value.i );

	err = ccs_hyperparameter_get_name(hyperparameter, &name);
	assert( err == CCS_SUCCESS );
	assert( strcmp(name, "my_param") == 0 );

	err = ccs_hyperparameter_get_user_data(hyperparameter, &user_data);
	assert( err == CCS_SUCCESS );
	assert( user_data == (void *)0xdeadbeef );

	err = ccs_hyperparameter_get_default_distribution(hyperparameter, &distribution);
	assert( err == CCS_SUCCESS );
	assert( distribution );

	err = ccs_distribution_get_type(distribution, &dist_type);
	assert( err == CCS_SUCCESS );
	assert( dist_type == CCS_UNIFORM );

	err = ccs_distribution_get_bounds(distribution, &interval);
	assert( err == CCS_SUCCESS );
	assert( interval.type == CCS_NUM_INTEGER );
	assert( interval.lower.i  == 0 );
	assert( interval.lower_included == CCS_TRUE );
	assert( interval.upper.i  == 4 );
	assert( interval.upper_included == CCS_FALSE );

	for(size_t i = 0; i < num_possible_values; i++) {
		err = ccs_hyperparameter_check_value(hyperparameter, possible_values[i],
		                                     &check);
		assert( err == CCS_SUCCESS );
		assert( check == CCS_TRUE );
	}

	default_value.type = CCS_FLOAT;
	err = ccs_hyperparameter_check_value(hyperparameter, default_value, &check);
	assert( err == CCS_SUCCESS );
	assert( check == CCS_FALSE );

	err = ccs_release_object(distribution);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );
}

void test_create_error() {
	ccs_hyperparameter_t       hyperparameter;
	ccs_error_t                err;
	const size_t               num_possible_values = 4;
	ccs_datum_t                possible_values[num_possible_values];
	const size_t               default_value_index = 2;

	for(size_t i = 0; i < num_possible_values; i++) {
		possible_values[i].type = CCS_INTEGER;
		possible_values[i].value.i = (i+1)*2;
	}

	possible_values[0].value.i = possible_values[num_possible_values-1].value.i;

	err = ccs_create_ordinal_hyperparameter("my_param", num_possible_values,
	                                        possible_values, default_value_index,
	                                        NULL,
	                                        &hyperparameter);
	assert( err == -CCS_INVALID_VALUE );
}

void test_samples() {
	ccs_rng_t                  rng;
	ccs_hyperparameter_t       hyperparameter;
	ccs_distribution_t         distribution;
	const size_t               num_samples = 10000;
	ccs_datum_t                samples[num_samples];
	ccs_error_t                err;
	const size_t               num_possible_values = 4;
	ccs_datum_t                possible_values[num_possible_values];
	const size_t               default_value_index = 2;

	for(size_t i = 0; i < num_possible_values; i++) {
		possible_values[i].type = CCS_INTEGER;
		possible_values[i].value.i = (i+1)*2;
	}

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_ordinal_hyperparameter("my_param", num_possible_values,
	                                        possible_values, default_value_index,
	                                        NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_hyperparameter_get_default_distribution(hyperparameter, &distribution);
	assert( err == CCS_SUCCESS );
	assert( distribution );

	err = ccs_hyperparameter_samples(hyperparameter, distribution, rng,
	                                 num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
		assert( samples[i].type == CCS_INTEGER );
		assert( samples[i].value.i %2 == 0 );
		assert( samples[i].value.i >= 2 );
		assert( samples[i].value.i <= (ccs_int_t)num_possible_values * 2);
	}

	err = ccs_release_object(distribution);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

void test_oversampling() {
	ccs_rng_t                  rng;
	ccs_hyperparameter_t       hyperparameter;
	ccs_distribution_t         distribution;
	const size_t               num_samples = 10000;
	ccs_datum_t                samples[num_samples];
	ccs_error_t                err;
	const size_t               num_possible_values = 4;
	ccs_datum_t                possible_values[num_possible_values];
	const size_t               default_value_index = 2;

	for(size_t i = 0; i < num_possible_values; i++) {
		possible_values[i].type = CCS_INTEGER;
		possible_values[i].value.i = (i+1)*2;
	}

	err = ccs_rng_create(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_int_distribution(0, num_possible_values+1,
	                                          CCS_LINEAR, 0, &distribution);
	assert( err == CCS_SUCCESS );

	err = ccs_create_ordinal_hyperparameter("my_param", num_possible_values,
	                                        possible_values, default_value_index,
	                                        NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_hyperparameter_samples(hyperparameter, distribution, rng,
	                                 num_samples, samples);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < num_samples; i++) {
		assert( samples[i].type == CCS_INTEGER );
		assert( samples[i].value.i % 2 == 0 );
		assert( samples[i].value.i >= 2 );
		assert( samples[i].value.i <= (ccs_int_t)num_possible_values * 2);
	}

	err = ccs_release_object(distribution);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
}

void test_compare() {
	ccs_hyperparameter_t       hyperparameter;
	ccs_error_t                err;
	const size_t               num_possible_values = 4;
	ccs_datum_t                possible_values[num_possible_values];
	const size_t               default_value_index = 2;
	ccs_int_t                  comp = 0;
	ccs_datum_t                invalid;

	invalid.value.i = -1;
	invalid.type = CCS_INTEGER;
	for(size_t i = 0; i < num_possible_values; i++) {
		possible_values[i].value.i = (i+1)*2;
		possible_values[i].type = CCS_INTEGER;
	}

	err = ccs_create_ordinal_hyperparameter("my_param", num_possible_values,
	                                        possible_values, default_value_index,
	                                        NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_ordinal_hyperparameter_compare_values(
		hyperparameter,
		possible_values[num_possible_values-1],
		possible_values[0], &comp);
	assert( err == CCS_SUCCESS );
	assert( comp == 1 );

	err = ccs_ordinal_hyperparameter_compare_values(
		hyperparameter, possible_values[0], 
		possible_values[num_possible_values-1],
		&comp);
	assert( err == CCS_SUCCESS );
	assert( comp == -1 );

	err = ccs_ordinal_hyperparameter_compare_values(
		hyperparameter,
		possible_values[0], 
		possible_values[0], &comp);
	assert( err == CCS_SUCCESS );
	assert( comp == 0 );

	err = ccs_ordinal_hyperparameter_compare_values(
		hyperparameter, invalid,
		possible_values[0], &comp);
	assert( err == -CCS_INVALID_VALUE );

	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );
}

void test_compare_float() {
	ccs_hyperparameter_t       hyperparameter;
	ccs_error_t                err;
	const size_t               num_possible_values = 4;
	ccs_datum_t                possible_values[num_possible_values];
	const size_t               default_value_index = 2;
	ccs_int_t                  comp = 0;
	ccs_datum_t                invalid;

	invalid.value.f = -0.0;
	invalid.type = CCS_FLOAT;
	possible_values[0].value.f = 1.0;
	possible_values[0].type = CCS_FLOAT;
	possible_values[1].value.f = -5.0;
	possible_values[1].type = CCS_FLOAT;
	possible_values[2].value.i = 5;
	possible_values[2].type = CCS_INTEGER;
	possible_values[3].value.f = 0.0;
	possible_values[3].type = CCS_FLOAT;

	err = ccs_create_ordinal_hyperparameter("my_param", num_possible_values,
	                                        possible_values, default_value_index,
	                                        NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_ordinal_hyperparameter_compare_values(
		hyperparameter,
		possible_values[num_possible_values-1],
		possible_values[0], &comp);
	assert( err == CCS_SUCCESS );
	assert( comp == 1 );

	err = ccs_ordinal_hyperparameter_compare_values(
		hyperparameter, possible_values[0], 
		possible_values[num_possible_values-1],
		&comp);
	assert( err == CCS_SUCCESS );
	assert( comp == -1 );

	err = ccs_ordinal_hyperparameter_compare_values(
		hyperparameter,
		possible_values[0], 
		possible_values[0], &comp);
	assert( err == CCS_SUCCESS );
	assert( comp == 0 );

	err = ccs_ordinal_hyperparameter_compare_values(
		hyperparameter, invalid,
		possible_values[0], &comp);
	assert( err == -CCS_INVALID_VALUE );

	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );
}

void test_compare_string() {
	ccs_hyperparameter_t       hyperparameter;
	ccs_error_t                err;
	const size_t               num_possible_values = 4;
	ccs_datum_t                possible_values[num_possible_values];
	const size_t               default_value_index = 2;
	ccs_int_t                  comp = 0;
	ccs_datum_t                invalid;

	invalid.value.s = "faraway";
	invalid.type = CCS_STRING;
	possible_values[0].value.s = "foo";
	possible_values[0].type = CCS_STRING;
	possible_values[1].value.s = "bar";
	possible_values[1].type = CCS_STRING;
	possible_values[2].value.i = 5;
	possible_values[2].type = CCS_INTEGER;
	possible_values[3].value.s = "baz";
	possible_values[3].type = CCS_STRING;

	err = ccs_create_ordinal_hyperparameter("my_param", num_possible_values,
	                                        possible_values, default_value_index,
	                                        NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_ordinal_hyperparameter_compare_values(
		hyperparameter,
		possible_values[num_possible_values-1],
		possible_values[0], &comp);
	assert( err == CCS_SUCCESS );
	assert( comp == 1 );

	err = ccs_ordinal_hyperparameter_compare_values(
		hyperparameter, possible_values[0], 
		possible_values[num_possible_values-1],
		&comp);
	assert( err == CCS_SUCCESS );
	assert( comp == -1 );

	err = ccs_ordinal_hyperparameter_compare_values(
		hyperparameter,
		possible_values[0], 
		possible_values[0], &comp);
	assert( err == CCS_SUCCESS );
	assert( comp == 0 );

	err = ccs_ordinal_hyperparameter_compare_values(
		hyperparameter, invalid,
		possible_values[0], &comp);
	assert( err == -CCS_INVALID_VALUE );

	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );
}

int main(int argc, char *argv[]) {
	ccs_init();
	test_create();
	test_create_error();
	test_samples();
	test_oversampling();
	test_compare();
	test_compare_float();
	test_compare_string();
	return 0;
}
