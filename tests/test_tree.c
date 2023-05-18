#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>

#define NUM_SAMPLES 20000

void
check_samples(
	size_t       num_areas,
	ccs_float_t *areas,
	int         *counts,
	size_t       num_samples,
	size_t      *samples)
{
	ccs_float_t sum     = 0.0;
	ccs_float_t inv_sum = 0.0;

	for (size_t i = 0; i < num_areas; i++)
		counts[i] = 0;
	for (size_t i = 0; i < num_areas; i++)
		sum += areas[i];
	inv_sum = 1.0 / sum;
	for (size_t i = 0; i < num_samples; i++) {
		assert(samples[i] < num_areas);
		counts[samples[i]]++;
	}
	for (size_t i = 0; i < num_areas; i++) {
		ccs_float_t target = num_samples * areas[i] * inv_sum;
		assert(counts[i] >= target * 0.95 &&
		       counts[i] <= target * 1.05);
	}
}

void
test_tree()
{
	ccs_tree_t  root, child, grand_child, parent, node, children[4];
	ccs_rng_t   rng;
	ccs_datum_t value, *values;
	size_t      arity, num_children, position_size, *position, index,
		samples[NUM_SAMPLES], num_values;
	int          counts[5];
	ccs_float_t  weight, bias, areas[5];
	ccs_bool_t   is_valid;
	ccs_result_t err = CCS_RESULT_SUCCESS;

	// Basic creation
	err              = ccs_create_tree(4, ccs_string("foo"), &root);
	assert(err == CCS_RESULT_SUCCESS);

	// Basic queries
	err = ccs_tree_get_value(root, &value);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!ccs_datum_cmp(value, ccs_string("foo")));

	err = ccs_tree_get_arity(root, &arity);
	assert(err == CCS_RESULT_SUCCESS);
	assert(arity == 4);

	for (size_t i = 0; i < 4; i++) {
		err = ccs_tree_get_child(root, i, &child);
		assert(err == CCS_RESULT_SUCCESS);
		assert(!child);
	}
	err = ccs_tree_get_child(root, 4, &child);
	assert(err == CCS_RESULT_ERROR_OUT_OF_BOUNDS);

	err = ccs_tree_get_children(root, 0, NULL, &num_children);
	assert(err == CCS_RESULT_SUCCESS);
	assert(num_children == 4);
	err = ccs_tree_get_children(root, 4, children, NULL);
	assert(err == CCS_RESULT_SUCCESS);
	for (size_t i = 0; i < 4; i++)
		assert(!children[i]);

	err = ccs_tree_get_parent(root, &parent, NULL);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!parent);

	err = ccs_tree_get_position(root, 0, NULL, &position_size);
	assert(err == CCS_RESULT_SUCCESS);
	assert(position_size == 0);

	err = ccs_tree_get_node_at_position(root, 0, NULL, &node);
	assert(err == CCS_RESULT_SUCCESS);
	assert(node == root);

	position    = (size_t *)malloc(sizeof(size_t));
	position[0] = 2;
	err         = ccs_tree_get_node_at_position(root, 1, position, &node);
	assert(err == CCS_RESULT_ERROR_INVALID_TREE);

	// Sampling related queries
	err = ccs_tree_get_weight(root, &weight);
	assert(err == CCS_RESULT_SUCCESS);
	assert(weight == 1.0);

	err = ccs_tree_get_bias(root, &bias);
	assert(err == CCS_RESULT_SUCCESS);
	assert(bias == 1.0);

	err = ccs_create_rng(&rng);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t i = 0; i < 100; i++) {
		err = ccs_tree_sample(root, rng, &index);
		assert(err == CCS_RESULT_SUCCESS);
		assert(index <= 4);
	}

	err = ccs_tree_samples(root, rng, NUM_SAMPLES, samples);
	assert(err == CCS_RESULT_SUCCESS);
	for (size_t i = 0; i < 5; i++)
		areas[i] = 1.0;
	check_samples(5, areas, counts, NUM_SAMPLES, samples);

	err = ccs_tree_set_weight(root, 2.0);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_tree_get_weight(root, &weight);
	assert(err == CCS_RESULT_SUCCESS);
	assert(weight == 2.0);

	err = ccs_tree_set_bias(root, 3.0);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_tree_get_bias(root, &bias);
	assert(err == CCS_RESULT_SUCCESS);
	assert(bias == 3.0);

	err = ccs_tree_samples(root, rng, NUM_SAMPLES, samples);
	assert(err == CCS_RESULT_SUCCESS);
	for (size_t i = 0; i < 4; i++)
		areas[i] = 1.0;
	areas[4] = 2.0;
	check_samples(5, areas, counts, NUM_SAMPLES, samples);

	// Treee growth
	err = ccs_create_tree(3, ccs_string("bar"), &child);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_tree_set_child(root, 2, child);
	assert(err == CCS_RESULT_SUCCESS);

	position[0] = 2;
	err         = ccs_tree_get_node_at_position(root, 1, position, &node);
	assert(err == CCS_RESULT_SUCCESS);
	assert(child == node);
	free(position);

	// weight of child subtree is 4.0 for now (1 + 3)
	areas[2] = 4.0;
	err      = ccs_tree_samples(root, rng, NUM_SAMPLES, samples);
	assert(err == CCS_RESULT_SUCCESS);
	check_samples(5, areas, counts, NUM_SAMPLES, samples);

	// Test bias
	err = ccs_tree_set_bias(child, 0.5);
	assert(err == CCS_RESULT_SUCCESS);
	// (1 + 3) / 2
	areas[2] = 2.0;
	err      = ccs_tree_samples(root, rng, NUM_SAMPLES, samples);
	assert(err == CCS_RESULT_SUCCESS);
	check_samples(5, areas, counts, NUM_SAMPLES, samples);

	// Test recursion
	err = ccs_create_tree(1, ccs_string("baz"), &grand_child);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_tree_set_child(child, 1, grand_child);
	assert(err == CCS_RESULT_SUCCESS);

	position    = (size_t *)malloc(2 * sizeof(size_t));
	position[0] = 2;
	position[1] = 1;
	err         = ccs_tree_position_is_valid(root, 2, position, &is_valid);
	assert(err == CCS_RESULT_SUCCESS);
	assert(is_valid == CCS_TRUE);
	err = ccs_tree_get_node_at_position(root, 2, position, &node);
	assert(err == CCS_RESULT_SUCCESS);
	assert(grand_child == node);
	free(position);

	position    = (size_t *)malloc(3 * sizeof(size_t));
	position[0] = 2;
	position[1] = 1;
	position[2] = 3;
	err         = ccs_tree_position_is_valid(root, 3, position, &is_valid);
	assert(err == CCS_RESULT_SUCCESS);
	assert(is_valid == CCS_FALSE);
	free(position);

	position    = (size_t *)malloc(2 * sizeof(size_t));
	position[0] = 2;
	position[1] = 1;
	values      = (ccs_datum_t *)malloc(sizeof(ccs_datum_t) * 3);
	err = ccs_tree_get_values_at_position(root, 2, position, 3, values);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!ccs_datum_cmp(values[0], ccs_string("foo")));
	assert(!ccs_datum_cmp(values[1], ccs_string("bar")));
	assert(!ccs_datum_cmp(values[2], ccs_string("baz")));
	free(position);

	position    = (size_t *)malloc(3 * sizeof(size_t));
	position[0] = 0;
	position[1] = 0;
	err = ccs_tree_get_position(grand_child, 3, position, &position_size);
	assert(err == CCS_RESULT_SUCCESS);
	assert(position_size == 2);
	assert(position[0] == 2);
	assert(position[1] == 1);
	free(position);

	values[0] = ccs_none;
	values[1] = ccs_none;
	values[2] = ccs_none;
	err       = ccs_tree_get_values(grand_child, 3, values, &num_values);
	assert(err == CCS_RESULT_SUCCESS);
	assert(num_values == 3);
	assert(!ccs_datum_cmp(values[0], ccs_string("foo")));
	assert(!ccs_datum_cmp(values[1], ccs_string("bar")));
	assert(!ccs_datum_cmp(values[2], ccs_string("baz")));
	free(values);

	// (1 + (1 + (1 + 1) + 1)) / 2
	areas[2] = 2.5;
	err      = ccs_tree_samples(root, rng, NUM_SAMPLES, samples);
	assert(err == CCS_RESULT_SUCCESS);
	check_samples(5, areas, counts, NUM_SAMPLES, samples);

	err = ccs_tree_set_weight(grand_child, 2.0);
	assert(err == CCS_RESULT_SUCCESS);
	// (1 + (1 + (2 + 1) + 1)) / 2
	areas[2] = 3.0;
	err      = ccs_tree_samples(root, rng, NUM_SAMPLES, samples);
	assert(err == CCS_RESULT_SUCCESS);
	check_samples(5, areas, counts, NUM_SAMPLES, samples);

	err = ccs_tree_set_bias(grand_child, 0.5);
	assert(err == CCS_RESULT_SUCCESS);
	// (1 + (1 + (2 + 1)/2 + 1)) / 2
	areas[2] = 2.25;
	err      = ccs_tree_samples(root, rng, NUM_SAMPLES, samples);
	assert(err == CCS_RESULT_SUCCESS);
	check_samples(5, areas, counts, NUM_SAMPLES, samples);

	char  *buff;
	size_t buff_size;

	err = ccs_object_serialize(
		root, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE,
		&buff_size, CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		root, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(root);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(child);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(grand_child);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&root, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	free(buff);

	err = ccs_tree_samples(root, rng, NUM_SAMPLES, samples);
	assert(err == CCS_RESULT_SUCCESS);
	check_samples(5, areas, counts, NUM_SAMPLES, samples);

	err = ccs_release_object(root);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(rng);
	assert(err == CCS_RESULT_SUCCESS);
}

int
main()
{
	ccs_init();
	test_tree();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
