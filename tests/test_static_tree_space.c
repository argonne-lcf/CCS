#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <cconfigspace.h>

#define NUM_SAMPLES 200000

void print_ccs_error_stack() {
	ccs_error_stack_t err;
	ccs_error_t code;
	const char *msg;
	size_t stack_depth;
	ccs_error_stack_elem_t *stack_elems;

	err = ccs_get_thread_error();
	if (!err)
		return;
	ccs_error_stack_get_code(err, &code);
	ccs_get_error_name(code, &msg);
	fprintf(stderr, "CCS Error: %s (%d): ", msg, code);
	ccs_error_stack_get_message(err, &msg);
	fprintf(stderr, "%s\n", msg);
	ccs_error_stack_get_elems(err, &stack_depth, &stack_elems);
	for (size_t i = 0; i < stack_depth; i++) {
		fprintf(stderr, "\t%s:%d:%s\n", stack_elems[i].file, stack_elems[i].line, stack_elems[i].func);
	}
	ccs_release_object(err);
}

void generate_tree(ccs_tree_t *tree, size_t depth, size_t rank) {
	ccs_error_t      err;
	ssize_t ar = depth - rank;
	size_t arity = (size_t)(ar < 0 ? 0 : ar);

	err = ccs_create_tree(arity, ccs_int(depth*100+rank), tree);
	assert( err == CCS_SUCCESS );
	for (size_t i = 0; i < arity; i++) {
		ccs_tree_t child;
		generate_tree(&child, depth - 1, i);
		err = ccs_tree_set_child(*tree, i, child);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(child);
		assert( err == CCS_SUCCESS );
	}
}

void test_static_tree_space() {
	ccs_error_t               err;
	ccs_tree_t                root, tree;
	ccs_tree_space_t          tree_space;
	ccs_tree_space_type_t     tree_type;
	ccs_rng_t                 rng, rng2;
	size_t                    position_size, position[4], depths[5];
	ccs_datum_t               value, values[5];
	ccs_float_t               areas[5] = {1.0, 4.0, 6.0, 8.0, 5.0};
	ccs_float_t               inv_sum;
	const char               *name;
	ccs_tree_configuration_t  config, configs[NUM_SAMPLES];


	generate_tree(&root, 4, 0);
	err = ccs_create_static_tree_space("space", root, &tree_space);
	assert( err == CCS_SUCCESS );

	err = ccs_tree_space_get_type(tree_space, &tree_type);
	assert( err == CCS_SUCCESS );
	assert( tree_type == CCS_TREE_SPACE_TYPE_STATIC );

	err = ccs_tree_space_get_name(tree_space, &name);
	assert( err == CCS_SUCCESS );
	assert( !strcmp(name, "space") );

	err = ccs_create_rng(&rng);
	assert( err == CCS_SUCCESS );
	err = ccs_tree_space_set_rng(tree_space, rng);
	assert( err == CCS_SUCCESS );

	err = ccs_tree_space_get_rng(tree_space, &rng2);
	assert( err == CCS_SUCCESS );
	assert( rng == rng2);

	err = ccs_tree_space_get_tree(tree_space, &tree);
	assert( err == CCS_SUCCESS );
	assert( tree == root );

	position[0] = 1;
	position[1] = 1;
	err = ccs_tree_space_get_node_at_position(tree_space, 2, position, &tree);
	assert( err == CCS_SUCCESS );
	err = ccs_tree_get_value(tree, &value);
	assert( err == CCS_SUCCESS );
	assert( value.value.i == 200 + 1 );

	err = ccs_tree_space_get_values_at_position(tree_space, 2, position, 3, values);
	assert( err == CCS_SUCCESS );
	assert( values[0].value.i == 400 + 0 );
	assert( values[1].value.i == 300 + 1 );
	assert( values[2].value.i == 200 + 1 );

	err = ccs_tree_space_sample(tree_space, &config);
	assert( err == CCS_SUCCESS );

	ccs_map_t    map;
	ccs_datum_t  d;
	char        *buff;
	size_t       buff_size;

	err = ccs_create_map(&map);
	assert( err == CCS_SUCCESS );

	err = ccs_object_serialize(config, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE, &buff_size, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	buff = (char *)malloc(buff_size);
	assert( buff );

	err = ccs_object_serialize(config, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(config);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&config, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_DESERIALIZE_OPTION_HANDLE_MAP, map, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_INVALID_HANDLE );

	d = ccs_object(tree_space);
	d.flags |= CCS_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(tree_space));
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&config, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_DESERIALIZE_OPTION_HANDLE_MAP, map, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );
	free(buff);

	err = ccs_release_object(map);
	assert( err == CCS_SUCCESS );

	err = ccs_tree_space_samples(tree_space, NUM_SAMPLES, configs);
	assert( err == CCS_SUCCESS );

	inv_sum = 0;
	for (size_t i = 0; i < 5; i++) {
		depths[i] = 0;
		inv_sum += areas[i];
	}
	inv_sum = 1.0/inv_sum;
	for (size_t i = 0; i < NUM_SAMPLES; i++) {
		ccs_bool_t is_valid;
		err = ccs_tree_space_check_configuration(tree_space, configs[i], &is_valid);
		assert( err == CCS_SUCCESS );
		assert( is_valid == CCS_TRUE );
		err = ccs_tree_configuration_get_position(configs[i], 0, NULL, &position_size);
		assert( err == CCS_SUCCESS );
		depths[position_size]++;
		err = ccs_release_object(configs[i]);
		assert( err == CCS_SUCCESS );
	}
	for (size_t i = 0; i < 5; i++) {
		ccs_float_t target = NUM_SAMPLES * areas[i] * inv_sum;
		assert( depths[i] >= target * 0.95 && depths[i] <= target * 1.05 );
	}

	err = ccs_object_serialize(tree_space, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE, &buff_size, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	buff = (char *)malloc(buff_size);
	assert( buff );

	err = ccs_object_serialize(tree_space, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(tree_space);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&tree_space, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );
	free(buff);

	err = ccs_tree_space_samples(tree_space, NUM_SAMPLES, configs);
	assert( err == CCS_SUCCESS );

	inv_sum = 0;
	for (size_t i = 0; i < 5; i++) {
		depths[i] = 0;
		inv_sum += areas[i];
	}
	inv_sum = 1.0/inv_sum;
	for (size_t i = 0; i < NUM_SAMPLES; i++) {
		ccs_bool_t is_valid;
		err = ccs_tree_space_check_configuration(tree_space, configs[i], &is_valid);
		assert( err == CCS_SUCCESS );
		assert( is_valid == CCS_TRUE );
		err = ccs_tree_configuration_get_position(configs[i], 0, NULL, &position_size);
		assert( err == CCS_SUCCESS );
		depths[position_size]++;
		err = ccs_release_object(configs[i]);
		assert( err == CCS_SUCCESS );
	}
	for (size_t i = 0; i < 5; i++) {
		ccs_float_t target = NUM_SAMPLES * areas[i] * inv_sum;
		assert( depths[i] >= target * 0.95 && depths[i] <= target * 1.05 );
	}

	err = ccs_release_object(config);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(rng);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(root);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(tree_space);
	assert( err == CCS_SUCCESS );
}

int main() {
	ccs_init();
	test_static_tree_space();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
