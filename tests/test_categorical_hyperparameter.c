#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

#define NUM_POSSIBLE_VALUES 4
#define NUM_SAMPLES 10000

void free_data(ccs_object_t o, void *user_data) {
	(void)o;
	free(user_data);
}

static void compare_hyperparameter(
		ccs_hyperparameter_t hyperparameter,
		const size_t         num_possible_values,
		ccs_datum_t          possible_values[],
		const size_t         default_value_index) {
	ccs_result_t               err;
	ccs_hyperparameter_type_t  type;
	ccs_datum_t                default_value;
	const char                *name;
	void *                     user_data;
	ccs_distribution_t         distribution;
	ccs_distribution_type_t    dist_type;
	ccs_interval_t             interval;
	ccs_bool_t                 check;

	err = ccs_hyperparameter_get_type(hyperparameter, &type);
	assert( err == CCS_SUCCESS );
	assert( type == CCS_HYPERPARAMETER_TYPE_CATEGORICAL );

	err = ccs_hyperparameter_get_default_value(hyperparameter, &default_value);
	assert( err == CCS_SUCCESS );
	assert( default_value.type == CCS_INTEGER );
	assert( default_value.value.i == possible_values[default_value_index].value.i );

	err = ccs_hyperparameter_get_name(hyperparameter, &name);
	assert( err == CCS_SUCCESS );
	assert( strcmp(name, "my_param") == 0 );

	err = ccs_object_get_user_data(hyperparameter, &user_data);
	assert( err == CCS_SUCCESS );
	assert( !strcmp((char *)user_data, "hello") );

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
}

ccs_result_t
serialize_callback(
		ccs_object_t  object,
		size_t        serialize_data_size,
		void         *serialize_data,
		size_t       *serialize_data_size_ret,
		void         *callback_user_data) {
	void *user_data;
	size_t sz;
	assert( callback_user_data == (void*)0xdeadbeef );
	ccs_result_t err = ccs_object_get_user_data(object, &user_data);
	assert( err == CCS_SUCCESS );
	assert(user_data);
	sz = strlen((char *)user_data) + 1;
	if (serialize_data_size_ret)
		*serialize_data_size_ret = sz;
	assert( !(serialize_data && serialize_data_size < sz) );
	if (serialize_data)
		strncpy((char *)serialize_data, (char *)user_data, sz);
	return CCS_SUCCESS;
}

ccs_result_t
deserialize_callback(
		ccs_object_t  object,
		size_t        serialize_data_size,
		void         *serialize_data,
		void         *callback_user_data) {
	assert( callback_user_data == (void*)0xbeefdead );
	assert( strlen((char *)serialize_data) + 1 == serialize_data_size );
	void *user_data = (void *)strdup((char *)serialize_data);
	ccs_result_t err = ccs_object_set_user_data(object, user_data);
	assert( err == CCS_SUCCESS );
	err = ccs_object_set_destroy_callback(object, &free_data, user_data);
	assert( err == CCS_SUCCESS );
	return CCS_SUCCESS;
}

void test_create() {
	ccs_hyperparameter_t  hyperparameter;
	ccs_result_t          err;
	void *                user_data;
	const size_t          num_possible_values = NUM_POSSIBLE_VALUES;
	ccs_datum_t           possible_values[NUM_POSSIBLE_VALUES];
	const size_t          default_value_index = 2;
	char                 *buff;
	size_t                buff_size;

	for(size_t i = 0; i < num_possible_values; i++) {
		possible_values[i].type = CCS_INTEGER;
		possible_values[i].value.i = (i+1)*2;
	}

	user_data = (void *)strdup("hello");

	err = ccs_create_categorical_hyperparameter("my_param", num_possible_values,
	                                            possible_values, default_value_index,
	                                            user_data, &hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_object_set_destroy_callback(hyperparameter, &free_data, user_data);
	assert( err == CCS_SUCCESS );

	err = ccs_object_set_serialize_callback(hyperparameter, serialize_callback, (void*)0xdeadbeef);
	assert( err == CCS_SUCCESS );

	compare_hyperparameter(hyperparameter, num_possible_values, possible_values, default_value_index);

	err = ccs_object_serialize(hyperparameter, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE, &buff_size);
	assert( err == CCS_SUCCESS );

	buff = (char *)malloc(buff_size);
	assert( buff );

	err = ccs_object_serialize(hyperparameter, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&hyperparameter, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_DESERIALIZE_CALLBACK, &deserialize_callback, (void*)0xbeefdead, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );
	free(buff);

	compare_hyperparameter(hyperparameter, num_possible_values, possible_values, default_value_index);

	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );
}

void test_samples() {
	ccs_rng_t                  rng;
	ccs_hyperparameter_t       hyperparameter;
	ccs_distribution_t         distribution;
	const size_t               num_samples = NUM_SAMPLES;
	ccs_datum_t                samples[NUM_SAMPLES];
	ccs_result_t               err;
	const size_t               num_possible_values = NUM_POSSIBLE_VALUES;
	ccs_datum_t                possible_values[NUM_POSSIBLE_VALUES];
	const size_t               default_value_index = 2;

	for(size_t i = 0; i < num_possible_values; i++) {
		possible_values[i].type = CCS_INTEGER;
		possible_values[i].value.i = (i+1)*2;
	}

	err = ccs_create_rng(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_categorical_hyperparameter("my_param", num_possible_values,
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
	const size_t               num_samples = NUM_SAMPLES;
	ccs_datum_t                samples[NUM_SAMPLES];
	ccs_result_t               err;
	const size_t               num_possible_values = NUM_POSSIBLE_VALUES;
	ccs_datum_t                possible_values[NUM_POSSIBLE_VALUES];
	const size_t               default_value_index = 2;

	for(size_t i = 0; i < num_possible_values; i++) {
		possible_values[i].type = CCS_INTEGER;
		possible_values[i].value.i = (i+1)*2;
	}

	err = ccs_create_rng(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_create_uniform_int_distribution(0, num_possible_values+1,
	                                          CCS_LINEAR, 0, &distribution);
	assert( err == CCS_SUCCESS );

	err = ccs_create_categorical_hyperparameter("my_param", num_possible_values,
	                                            possible_values, default_value_index,
	                                            NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );

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

int main() {
	ccs_init();
	test_create();
	test_samples();
	test_oversampling();
	ccs_fini();
	return 0;
}
