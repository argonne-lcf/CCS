#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <cconfigspace.h>

#define NUM_SAMPLES 200000

void
generate_tree(ccs_tree_t *tree, size_t depth, size_t rank)
{
	ccs_result_t err;
	ssize_t      ar    = depth - rank;
	size_t       arity = (size_t)(ar < 0 ? 0 : ar);

	err = ccs_create_tree(arity, ccs_int(depth * 100 + rank), tree);
	assert(err == CCS_RESULT_SUCCESS);
	for (size_t i = 0; i < arity; i++) {
		ccs_tree_t child;
		generate_tree(&child, depth - 1, i);
		err = ccs_tree_set_child(*tree, i, child);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(child);
		assert(err == CCS_RESULT_SUCCESS);
	}
}

void
test_static_tree_space(void)
{
	ccs_result_t             err;
	ccs_tree_t               root, tree;
	ccs_tree_space_t         tree_space;
	ccs_tree_space_type_t    tree_type;
	ccs_rng_t                rng, rng2;
	size_t                   position_size, *position, depths[5];
	ccs_datum_t              value, *values;
	ccs_float_t              areas[5] = {1.0, 4.0, 6.0, 8.0, 5.0};
	ccs_float_t              inv_sum;
	const char              *name;
	ccs_tree_configuration_t config, configs[NUM_SAMPLES];

	generate_tree(&root, 4, 0);
	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_static_tree_space(
		"space", root, NULL, rng, &tree_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_tree_space_get_type(tree_space, &tree_type);
	assert(err == CCS_RESULT_SUCCESS);
	assert(tree_type == CCS_TREE_SPACE_TYPE_STATIC);

	err = ccs_tree_space_get_name(tree_space, &name);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!strcmp(name, "space"));

	err = ccs_tree_space_get_rng(tree_space, &rng2);
	assert(err == CCS_RESULT_SUCCESS);
	assert(rng == rng2);

	err = ccs_tree_space_get_tree(tree_space, &tree);
	assert(err == CCS_RESULT_SUCCESS);
	assert(tree == root);

	position    = (size_t *)malloc(2 * sizeof(size_t));
	position[0] = 1;
	position[1] = 1;
	err         = ccs_tree_space_get_node_at_position(
                tree_space, 2, position, &tree);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_tree_get_value(tree, &value);
	assert(err == CCS_RESULT_SUCCESS);
	assert(value.value.i == 200 + 1);

	values = (ccs_datum_t *)malloc(sizeof(ccs_datum_t) * 3);
	err    = ccs_tree_space_get_values_at_position(
                tree_space, 2, position, 3, values);
	assert(err == CCS_RESULT_SUCCESS);
	assert(values[0].value.i == 400 + 0);
	assert(values[1].value.i == 300 + 1);
	assert(values[2].value.i == 200 + 1);
	free(values);
	free(position);

	err = ccs_tree_space_sample(tree_space, NULL, NULL, &config);
	assert(err == CCS_RESULT_SUCCESS);

	ccs_map_t   map;
	ccs_datum_t d;
	char       *buff;
	size_t      buff_size;

	err = ccs_create_map(&map);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_serialize(
		config, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		config, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(config);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&config, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_HANDLE);

	d = ccs_object(tree_space);
	d.flags |= CCS_DATUM_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(tree_space));
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&config, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	free(buff);

	err = ccs_release_object(map);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_tree_space_samples(
		tree_space, NULL, NULL, NUM_SAMPLES, configs);
	assert(err == CCS_RESULT_SUCCESS);

	inv_sum = 0;
	for (size_t i = 0; i < 5; i++) {
		depths[i] = 0;
		inv_sum += areas[i];
	}
	inv_sum = 1.0 / inv_sum;
	for (size_t i = 0; i < NUM_SAMPLES; i++) {
		ccs_bool_t is_valid;
		err = ccs_tree_space_check_configuration(
			tree_space, configs[i], &is_valid);
		assert(err == CCS_RESULT_SUCCESS);
		assert(is_valid == CCS_TRUE);
		err = ccs_tree_configuration_get_position(
			configs[i], 0, NULL, &position_size);
		assert(err == CCS_RESULT_SUCCESS);
		depths[position_size]++;
		err = ccs_release_object(configs[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
	for (size_t i = 0; i < 5; i++) {
		ccs_float_t target = NUM_SAMPLES * areas[i] * inv_sum;
		assert(depths[i] >= target * 0.95 &&
		       depths[i] <= target * 1.05);
	}

	err = ccs_object_serialize(
		tree_space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		tree_space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(tree_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&tree_space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	free(buff);

	err = ccs_tree_space_samples(
		tree_space, NULL, NULL, NUM_SAMPLES, configs);
	assert(err == CCS_RESULT_SUCCESS);

	inv_sum = 0;
	for (size_t i = 0; i < 5; i++) {
		depths[i] = 0;
		inv_sum += areas[i];
	}
	inv_sum = 1.0 / inv_sum;
	for (size_t i = 0; i < NUM_SAMPLES; i++) {
		ccs_bool_t is_valid;
		err = ccs_tree_space_check_configuration(
			tree_space, configs[i], &is_valid);
		assert(err == CCS_RESULT_SUCCESS);
		assert(is_valid == CCS_TRUE);
		err = ccs_tree_configuration_get_position(
			configs[i], 0, NULL, &position_size);
		assert(err == CCS_RESULT_SUCCESS);
		depths[position_size]++;
		err = ccs_release_object(configs[i]);
		assert(err == CCS_RESULT_SUCCESS);
	}
	for (size_t i = 0; i < 5; i++) {
		ccs_float_t target = NUM_SAMPLES * areas[i] * inv_sum;
		assert(depths[i] >= target * 0.95 &&
		       depths[i] <= target * 1.05);
	}

	err = ccs_release_object(config);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(root);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(tree_space);
	assert(err == CCS_RESULT_SUCCESS);
}

int
main(void)
{
	ccs_init();
	test_static_tree_space();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
