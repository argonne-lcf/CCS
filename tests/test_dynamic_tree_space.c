#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <cconfigspace.h>

#define NUM_SAMPLES 20000

static ccs_result_t
my_tree_del(ccs_tree_space_t tree_space)
{
	(void)tree_space;
	return CCS_RESULT_SUCCESS;
}

static ccs_result_t
my_tree_get_child(
	ccs_tree_space_t tree_space,
	ccs_tree_t       parent,
	size_t           child_index,
	ccs_tree_t      *child_ret)
{
	(void)tree_space;
	ccs_result_t err;
	size_t       depth;
	err                = ccs_tree_get_position(parent, 0, NULL, &depth);
	size_t child_depth = depth + 1;
	assert(err == CCS_RESULT_SUCCESS);
	ssize_t ar    = 4 - child_depth - child_index;
	size_t  arity = (size_t)(ar < 0 ? 0 : ar);

	err           = ccs_create_tree(
                arity, ccs_int((4 - child_depth) * 100 + child_index),
                child_ret);
	assert(err == CCS_RESULT_SUCCESS);
	return CCS_RESULT_SUCCESS;
}

void
test_dynamic_tree_space(void)
{
	ccs_result_t                    err;
	ccs_bool_t                      is_valid;
	ccs_tree_t                      root, tree;
	ccs_tree_space_t                tree_space;
	ccs_tree_space_type_t           tree_type;
	ccs_rng_t                       rng, rng2;
	size_t                          position_size, *position, depths[5];
	ccs_datum_t                     value, *values;
	ccs_float_t                     areas[5] = {1.0, 4.0, 2.0, 1.0, 0.0};
	ccs_float_t                     inv_sum;
	const char                     *name;
	ccs_tree_configuration_t        config, configs[NUM_SAMPLES];

	ccs_dynamic_tree_space_vector_t vector = {
		&my_tree_del, &my_tree_get_child, NULL, NULL};
	err = ccs_create_tree(4, ccs_int(4 * 100), &root);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_create_dynamic_tree_space(
		"space", root, NULL, rng, &vector, NULL, &tree_space);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_tree_space_get_type(tree_space, &tree_type);
	assert(err == CCS_RESULT_SUCCESS);
	assert(tree_type == CCS_TREE_SPACE_TYPE_DYNAMIC);

	err = ccs_tree_space_get_name(tree_space, &name);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!strcmp(name, "space"));

	err = ccs_tree_space_get_rng(tree_space, &rng2);
	assert(err == CCS_RESULT_SUCCESS);
	assert(rng == rng2);

	err = ccs_tree_space_get_tree(tree_space, &tree);
	assert(err == CCS_RESULT_SUCCESS);
	assert(tree == root);

	err = ccs_tree_space_get_node_at_position(tree_space, 0, NULL, &tree);
	assert(err == CCS_RESULT_SUCCESS);
	assert(tree == root);
	values = (ccs_datum_t *)malloc(sizeof(ccs_datum_t));
	err    = ccs_tree_space_get_values_at_position(
                tree_space, 0, NULL, 1, values);
	assert(err == CCS_RESULT_SUCCESS);
	assert(values[0].value.i == 400 + 0);
	free(values);
	err = ccs_tree_space_check_position(tree_space, 0, NULL, &is_valid);
	assert(err == CCS_RESULT_SUCCESS);
	assert(is_valid == CCS_TRUE);

	position    = (size_t *)malloc(2 * sizeof(size_t));
	position[0] = 1;
	position[0] = 4;
	err = ccs_tree_space_check_position(tree_space, 2, position, &is_valid);
	assert(err == CCS_RESULT_SUCCESS);
	assert(is_valid == CCS_FALSE);

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

	err = ccs_tree_space_samples(
		tree_space, NULL, NULL, NUM_SAMPLES, configs);
	assert(err == CCS_RESULT_SUCCESS);

	char  *buff;
	size_t buff_size;

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

	inv_sum = 0;
	for (size_t i = 0; i < 5; i++) {
		depths[i] = 0;
		inv_sum += areas[i];
	}
	inv_sum = 1.0 / inv_sum;
	for (size_t i = 0; i < NUM_SAMPLES; i++) {
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

	err = ccs_object_deserialize(
		(ccs_object_t *)&tree_space, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_VECTOR, &vector,
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

	err = ccs_release_object(tree_space);
	assert(err == CCS_RESULT_SUCCESS);
}

int
main(void)
{
	ccs_init();
	test_dynamic_tree_space();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
