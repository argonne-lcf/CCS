#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

double d = -2.0;

ccs_hyperparameter_t create_dummy_hyperparameter(const char * name) {
	ccs_hyperparameter_t hyperparameter;
	ccs_result_t         err;
	err = ccs_create_numerical_hyperparameter(name, CCS_NUM_FLOAT,
	                                          CCSF(-5.0), CCSF(5.0),
	                                          CCSF(0.0), CCSF(d),
	                                          NULL, &hyperparameter);
	d += 1.0;
	if (d >= 5.0)
		d = -5.0;
	assert( err == CCS_SUCCESS );
	return hyperparameter;
}

void test_create() {
	ccs_features_space_t  features_space;
	ccs_result_t           err;
	ccs_object_type_t      type;
	const char            *name;
	void *                 user_data;
	size_t                 sz;

	err = ccs_create_features_space("my_features_space", (void *)0xdeadbeef,
	                                &features_space);
	assert( err == CCS_SUCCESS );

	err = ccs_object_get_type(features_space, &type);
	assert( err == CCS_SUCCESS );
	assert( type == CCS_FEATURES_SPACE );

	err = ccs_features_space_get_name(features_space, &name);
	assert( err == CCS_SUCCESS );
	assert( strcmp(name, "my_features_space") == 0 );

	err = ccs_features_space_get_user_data(features_space, &user_data);
	assert( err == CCS_SUCCESS );
	assert( user_data == (void *)0xdeadbeef );

	err = ccs_features_space_get_num_hyperparameters(features_space, &sz);
	assert( err == CCS_SUCCESS );
	assert( sz == 0 );

	err = ccs_release_object(features_space);
	assert( err == CCS_SUCCESS );
}

void check_features(ccs_features_space_t  features_space,
                   size_t                 sz,
                   ccs_hyperparameter_t  *hyperparameters) {
	ccs_hyperparameter_t  hyperparameter;
	ccs_result_t          err;
	size_t                sz_ret;
	size_t                index;
	ccs_hyperparameter_t *hyperparameters_ret =
		(ccs_hyperparameter_t *)malloc(sizeof(ccs_hyperparameter_t)*(sz+1));
	const char           *name;

	err = ccs_features_space_get_num_hyperparameters(features_space,
	                                                 &sz_ret);
	assert( err == CCS_SUCCESS );
	assert( sz_ret == sz );

	for (size_t i = 0; i < sz; i++) {
		err = ccs_features_space_get_hyperparameter(features_space, i,
		                                            &hyperparameter);
		assert( err == CCS_SUCCESS );
		assert( hyperparameter == hyperparameters[i] );
		err = ccs_features_space_get_hyperparameter_index(
			features_space, hyperparameter, &index);
		assert( err == CCS_SUCCESS );
		assert( index == i);
	}
	err = ccs_features_space_get_hyperparameters(features_space, sz + 1,
	                                             hyperparameters_ret, &sz_ret);
	assert( err == CCS_SUCCESS );
	assert( sz_ret == sz );
	for (size_t i = 0; i < sz; i++)
		assert( hyperparameters_ret[i] == hyperparameters[i] );
	assert( hyperparameters_ret[sz] == NULL );

	for (size_t i = 0; i < sz; i++) {
		err = ccs_hyperparameter_get_name(hyperparameters[i], &name);
		assert( err == CCS_SUCCESS );
		err = ccs_features_space_get_hyperparameter_by_name(
			features_space, name, &hyperparameter);
		assert( err == CCS_SUCCESS );
		assert( hyperparameter == hyperparameters[i] );
	}

	free(hyperparameters_ret);
}

void test_add() {
	ccs_hyperparameter_t hyperparameters[3];
	ccs_features_space_t features_space;
	ccs_result_t         err;

	err = ccs_create_features_space("my_features_space", NULL,
	                                &features_space);
	assert( err == CCS_SUCCESS );

	hyperparameters[0] = create_dummy_hyperparameter("param1");
	hyperparameters[1] = create_dummy_hyperparameter("param2");
	hyperparameters[2] = create_dummy_hyperparameter("param3");

	err = ccs_features_space_add_hyperparameter(features_space,
	                                            hyperparameters[0]);
	assert( err == CCS_SUCCESS );

	err = ccs_features_space_add_hyperparameter(features_space,
	                                            hyperparameters[0]);
	assert( err == -CCS_INVALID_HYPERPARAMETER );

	err = ccs_features_space_add_hyperparameter(features_space,
	                                            hyperparameters[1]);
	assert( err == CCS_SUCCESS );
	err = ccs_features_space_add_hyperparameter(features_space,
	                                            hyperparameters[2]);
	assert( err == CCS_SUCCESS );

	check_features(features_space, 3, hyperparameters);

	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(hyperparameters[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(features_space);
	assert( err == CCS_SUCCESS );
}

void test_add_list() {
	ccs_hyperparameter_t hyperparameters[3];
	ccs_features_space_t features_space;
	ccs_result_t         err;

	err = ccs_create_features_space("my_config_space", NULL,
	                                &features_space);
	assert( err == CCS_SUCCESS );

	hyperparameters[0] = create_dummy_hyperparameter("param1");
	hyperparameters[1] = create_dummy_hyperparameter("param2");
	hyperparameters[2] = create_dummy_hyperparameter("param3");

	err = ccs_features_space_add_hyperparameters(features_space, 3,
	                                             hyperparameters);
	assert( err == CCS_SUCCESS );

	check_features(features_space, 3, hyperparameters);

	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(hyperparameters[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(features_space);
	assert( err == CCS_SUCCESS );
}

int main(int argc, char *argv[]) {
	ccs_init();
	test_create();
	test_add();
	test_add_list();
	return 0;
}
