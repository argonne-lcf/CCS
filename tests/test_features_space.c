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

	err = ccs_object_get_user_data(features_space, &user_data);
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

void test_features() {
	ccs_hyperparameter_t  hyperparameters[3];
	ccs_features_space_t  features_space, features_space_ret;
	ccs_datum_t           values[3] =
		{ ccs_float(-1.0), ccs_float(0.0), ccs_float(1.0) };
	ccs_datum_t           values_ret[3];
	ccs_features_t        features1, features2;
	ccs_result_t          err;
	void                 *user_data_ret;
	ccs_datum_t           datum;
	size_t                num_values_ret;
	int                   cmp;

	err = ccs_create_features_space("my_config_space", NULL,
	                                &features_space);
	assert( err == CCS_SUCCESS );

	hyperparameters[0] = create_dummy_hyperparameter("param1");
	hyperparameters[1] = create_dummy_hyperparameter("param2");
	hyperparameters[2] = create_dummy_hyperparameter("param3");

	err = ccs_features_space_add_hyperparameters(features_space, 3,
	                                             hyperparameters);
	assert( err == CCS_SUCCESS );

	err = ccs_create_features(features_space, 3, values,
	                          (void*)0xdeadbeef, &features1);
	assert( err == CCS_SUCCESS );

	err = ccs_features_get_features_space(features1, &features_space_ret);
	assert( err == CCS_SUCCESS );
	assert( features_space == features_space_ret );

	err = ccs_object_get_user_data(features1, &user_data_ret);
	assert( err == CCS_SUCCESS );
	assert( (void*)0xdeadbeef == user_data_ret );

	for (size_t i = 0; i < 3; i++) {
		err = ccs_features_get_value(features1, i, &datum);
		assert( err == CCS_SUCCESS );
		assert( values[i].type == datum.type );
		assert( values[i].value.f == datum.value.f );
	}

	err = ccs_features_get_values(features1, 3, values_ret, &num_values_ret);
	assert( err == CCS_SUCCESS );
	assert( 3 == num_values_ret );
	for (size_t i = 0; i < 3; i++) {
		assert( values[i].type == values_ret[i].type );
		assert( values[i].value.f == values_ret[i].value.f );
	}

	err = ccs_features_get_value_by_name(features1, "param2", &datum);
	assert( err == CCS_SUCCESS );
	assert( values[1].type == datum.type );
	assert( values[1].value.f == datum.value.f );

	err = ccs_features_check(features1);
	assert( err == CCS_SUCCESS );

	err = ccs_features_space_check_features(features_space, features1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_features(features_space, 3, values,
	                          (void*)0xdeadbeef, &features2);

	assert( err == CCS_SUCCESS );

	err = ccs_features_cmp(features1, features2, &cmp);
	assert( err == CCS_SUCCESS );
	assert( 0 == cmp );

	err = ccs_features_set_value(features2, 1, ccs_float(0.5));
	assert( err == CCS_SUCCESS );

	err = ccs_features_cmp(features1, features2, &cmp);
	assert( err == CCS_SUCCESS );
	assert( 0 > cmp);

	err = ccs_features_set_value(features2, 1, ccs_float(10.0));
	assert( err == CCS_SUCCESS );

	err = ccs_features_space_check_features(features_space, features2);
	assert( err == -CCS_INVALID_FEATURES );

	err = ccs_features_space_check_features(features_space, features2);
	assert( err == -CCS_INVALID_FEATURES );

	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(hyperparameters[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(features_space);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(features1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(features2);
	assert( err == CCS_SUCCESS );
}

void test_deserialize() {
	ccs_hyperparameter_t  hyperparameters[3], hyperparameters_new[3];
	ccs_features_space_t  features_space;
	ccs_map_t             map;
	ccs_result_t          err;
	char                 *buff;
	size_t                buff_size;
	ccs_datum_t           d;
	ccs_bool_t            found;

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

	err = ccs_object_serialize(features_space, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_TYPE_SIZE, &buff_size);
	assert( err == CCS_SUCCESS );

	buff = (char *)malloc(buff_size);
	assert( buff );

	err = ccs_object_serialize(features_space, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_TYPE_MEMORY, buff_size, buff);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < 3; i++) {
		err = ccs_release_object(hyperparameters[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(features_space);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&features_space, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_TYPE_MEMORY, buff_size, buff, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_features_space_get_hyperparameter_by_name(features_space, "param1", &hyperparameters_new[0]);
	assert( err == CCS_SUCCESS );
	err = ccs_features_space_get_hyperparameter_by_name(features_space, "param2", &hyperparameters_new[1]);
	assert( err == CCS_SUCCESS );
	err = ccs_features_space_get_hyperparameter_by_name(features_space, "param3", &hyperparameters_new[2]);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(features_space);
	assert( err == CCS_SUCCESS );

	err = ccs_create_map(&map);
	assert( err == CCS_SUCCESS );
	err = ccs_object_deserialize((ccs_object_t*)&features_space, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_TYPE_MEMORY, buff_size, buff,
	                             CCS_DESERIALIZE_OPTION_HANDLE_MAP, map, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_features_space_get_hyperparameter_by_name(features_space, "param1", &hyperparameters_new[0]);
	assert( err == CCS_SUCCESS );
	err = ccs_features_space_get_hyperparameter_by_name(features_space, "param2", &hyperparameters_new[1]);
	assert( err == CCS_SUCCESS );
	err = ccs_features_space_get_hyperparameter_by_name(features_space, "param3", &hyperparameters_new[2]);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < 3; i++) {
		err = ccs_map_exist(map, ccs_object(hyperparameters[i]), &found);
		assert( err == CCS_SUCCESS );
		assert( found );
		err = ccs_map_get(map, ccs_object(hyperparameters[i]), &d);
		assert( err == CCS_SUCCESS );
		assert( d.type == CCS_OBJECT );
		assert( d.value.o == hyperparameters_new[i] );
	}
	
	err = ccs_release_object(map);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(features_space);
	assert( err == CCS_SUCCESS );
	free(buff);
}

int main() {
	ccs_init();
	test_create();
	test_add();
	test_add_list();
	test_features();
	test_deserialize();
	ccs_fini();
	return 0;
}
