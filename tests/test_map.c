#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>

void test_map() {
	ccs_map_t             map, map2;
	ccs_bool_t            found;
	ccs_datum_t           d1, d2, d3, d_ret, *keys, *values;
	size_t                d_count;
	ccs_error_t           err = CCS_SUCCESS;
	char                 *str1, *str2;
	ccs_hyperparameter_t  hp;
	char                 *buff;
	size_t                buff_size;

	err = ccs_create_map(&map);
	assert( err == CCS_SUCCESS );

	err = ccs_map_exist(map, ccs_int(10), &found);
	assert( err == CCS_SUCCESS );
	assert( found == CCS_FALSE );

	err = ccs_map_get(map, ccs_int(10), &d_ret);
	assert( err == CCS_SUCCESS );
	assert( !ccs_datum_cmp(d_ret, ccs_none) );

	err = ccs_map_set(map, ccs_string("hello"), ccs_float(1.0));
	assert( err == CCS_SUCCESS );

	err = ccs_map_exist(map, ccs_string("hello"), &found);
	assert( err == CCS_SUCCESS );
	assert( found == CCS_TRUE );

	err = ccs_map_get(map, ccs_string("hello"), &d_ret);
	assert( err == CCS_SUCCESS );
	assert( !ccs_datum_cmp(d_ret, ccs_float(1.0)) );

	str1 = (char *)malloc(strlen("foo") + 1);
	assert( str1 );
	strcpy(str1, "foo");

	str2 = (char *)malloc(strlen("bar") + 1);
	assert( str2 );
	strcpy(str2, "bar");

	d1 = ccs_string(str1);
	d1.flags |= CCS_FLAG_TRANSIENT;

	d2 = ccs_string(str2);
	d2.flags |= CCS_FLAG_TRANSIENT;

	err = ccs_map_set(map, d1, ccs_true);
	assert( err == CCS_SUCCESS );
	free(str1);

	err = ccs_map_get(map, ccs_string("foo"), &d_ret);
	assert( err == CCS_SUCCESS );
	assert( !ccs_datum_cmp(d_ret, ccs_true) );

	err = ccs_map_set(map, ccs_string("foo"), ccs_false);
	assert( err == CCS_SUCCESS );
	err = ccs_map_get(map, ccs_string("foo"), &d_ret);
	assert( err == CCS_SUCCESS );
	assert( !ccs_datum_cmp(d_ret, ccs_false) );

	err = ccs_map_set(map, ccs_false, d2);
	assert( err == CCS_SUCCESS );
	free(str2);

	err = ccs_map_get(map, ccs_false, &d_ret);
	assert( err == CCS_SUCCESS );
	assert( !ccs_datum_cmp(d_ret, ccs_string("bar")) );

	err = ccs_create_numerical_hyperparameter("my_param", CCS_NUM_FLOAT,
	                                          CCSF(-5.0), CCSF(5.0),
	                                          CCSF(0.0), CCSF(1.0),
	                                          &hp);
	assert( err == CCS_SUCCESS );

	err = ccs_map_set(map, ccs_object(hp), ccs_inactive);
	assert( err == CCS_SUCCESS );

	err = ccs_map_set(map, ccs_int(5), ccs_object(hp));
	assert( err == CCS_SUCCESS );
	err = ccs_map_set(map, ccs_int(5), ccs_object(hp));
	assert( err == CCS_SUCCESS );

	d3 = ccs_object((ccs_object_t)0xdeadbeef);
	d3.flags = CCS_FLAG_ID;
	err = ccs_map_set(map, ccs_int(6), d3);
	assert( err == CCS_SUCCESS );

	err = ccs_map_get(map, ccs_int(6), &d_ret);
	assert( err == CCS_SUCCESS );
	assert( d_ret.type == CCS_OBJECT );
	assert( d_ret.flags & CCS_FLAG_ID );
	assert( d_ret.value.o == (ccs_object_t)0xdeadbeef );

	err = ccs_map_del(map, ccs_int(5));
	assert( err == CCS_SUCCESS );

	err = ccs_map_get_keys(map, 0, NULL, &d_count);
	assert( err == CCS_SUCCESS );
	assert( d_count == 5 );
	keys = (ccs_datum_t *)calloc( d_count, sizeof(ccs_datum_t));
	assert( keys );
	err = ccs_map_get_keys(map, d_count, keys, NULL);
	assert( err == CCS_SUCCESS );

	err = ccs_map_get_values(map, 0, NULL, &d_count);
	assert( err == CCS_SUCCESS );
	assert( d_count == 5 );
	values = (ccs_datum_t *)calloc( d_count, sizeof(ccs_datum_t));
	assert( values );
	err = ccs_map_get_values(map, d_count, values, NULL);
	assert( err == CCS_SUCCESS );

	err = ccs_map_get_pairs(map, 0, NULL, NULL, &d_count);
	assert( err == CCS_SUCCESS );
	assert( d_count == 5 );
	err = ccs_map_get_pairs(map, d_count, keys, values, NULL);
	assert( err == CCS_SUCCESS );

	err = ccs_object_serialize(map, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE, &buff_size, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	buff = (char *)malloc(buff_size);
	assert( buff );

	err = ccs_object_serialize(map, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&map2, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );
	free(buff);

	err = ccs_map_get_pairs(map2, 0, NULL, NULL, &d_count);
	assert( err == CCS_SUCCESS );
	assert( d_count == 5 );

	for (size_t i = 0; i < d_count; i++) {
		err = ccs_map_get(map, keys[i], &d_ret);
		assert( err == CCS_SUCCESS );
		assert( !ccs_datum_cmp(values[i], d_ret) );
	}

	free(keys);
	free(values);

	err = ccs_map_clear(map);
	assert( err == CCS_SUCCESS );

	err = ccs_map_get_pairs(map, 0, NULL, NULL, &d_count);
	assert( err == CCS_SUCCESS );
	assert( d_count == 0 );

	err = ccs_release_object(map2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hp);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(map);
	assert( err == CCS_SUCCESS );
}

int main() {
	ccs_init();
	test_map();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
