#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>
#include "test_utils.h"

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
create_tree_tuning_problem(
	ccs_tree_space_t      *tree_space,
	ccs_objective_space_t *ospace)
{
	ccs_parameter_t      parameter;
	ccs_tree_t           root;
	ccs_expression_t     expression;
	ccs_objective_type_t otype;
	ccs_result_t         err;

	generate_tree(&root, 5, 0);
	err = ccs_create_static_tree_space("space", root, NULL, tree_space);
	assert(err == CCS_RESULT_SUCCESS);

	parameter = create_numerical("sum", -CCS_INFINITY, CCS_INFINITY);
	err       = ccs_create_variable(parameter, &expression);
	assert(err == CCS_RESULT_SUCCESS);
	otype = CCS_OBJECTIVE_TYPE_MAXIMIZE;

	err   = ccs_create_objective_space(
                "ospace", (ccs_search_space_t)*tree_space, 1, &parameter, 1,
                &expression, &otype, ospace);
	print_ccs_error_stack();
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(root);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test(void)
{
	ccs_tree_space_t      tree_space;
	ccs_objective_space_t ospace;
	ccs_tuner_t           tuner, tuner_copy;
	ccs_result_t          err;
	ccs_datum_t           d;
	char                 *buff;
	size_t                buff_size;
	ccs_map_t             map;

	create_tree_tuning_problem(&tree_space, &ospace);

	err = ccs_create_random_tuner("problem", ospace, &tuner);
	assert(err == CCS_RESULT_SUCCESS);

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t                values[6], res;
		size_t                     num_values;
		ccs_search_configuration_t configuration;
		ccs_evaluation_t           evaluation;
		ccs_int_t                  v;
		err = ccs_tuner_ask(tuner, 1, &configuration, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_tree_configuration_get_values(
			(ccs_tree_configuration_t)configuration, 6, values,
			&num_values);
		assert(err == CCS_RESULT_SUCCESS);
		v = 0;
		for (size_t i = 0; i < num_values; i++) {
			v += values[i].value.i;
		}
		res = ccs_float(v);
		err = ccs_create_evaluation(
			ospace, configuration, CCS_RESULT_SUCCESS, 1, &res,
			&evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_tuner_tell(tuner, 1, &evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(configuration);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(evaluation);
		assert(err == CCS_RESULT_SUCCESS);
	}

	size_t           count;
	ccs_evaluation_t history[100];
	ccs_datum_t      max = ccs_float(-INFINITY);
	err = ccs_tuner_get_history(tuner, 100, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 100);

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t res;
		err = ccs_evaluation_get_objective_value(history[i], 0, &res);
		assert(err == CCS_RESULT_SUCCESS);
		if (res.value.f > max.value.f)
			max.value.f = res.value.f;
	}

	ccs_evaluation_t evaluation;
	ccs_datum_t      res;
	err = ccs_tuner_get_optima(tuner, 1, &evaluation, NULL);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_evaluation_get_objective_value(evaluation, 0, &res);
	assert(res.value.f == max.value.f);

	/* Test (de)serialization */
	err = ccs_create_map(&map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_object_serialize(
		tuner, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	buff = (char *)malloc(buff_size);
	assert(buff);

	err = ccs_object_serialize(
		tuner, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&tuner_copy, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_tuner_get_history(tuner_copy, 100, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 100);

	err = ccs_tuner_get_optima(tuner_copy, 1, &evaluation, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 1);

	err = ccs_map_get(map, ccs_object((ccs_object_t)tuner), &d);
	assert(err == CCS_RESULT_SUCCESS);
	assert(d.type == CCS_DATA_TYPE_OBJECT);
	assert(d.value.o == (ccs_object_t)tuner_copy);

	free(buff);
	err = ccs_release_object(map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(tuner_copy);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(ospace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(tree_space);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(tuner);
	assert(err == CCS_RESULT_SUCCESS);
}

void
test_tree_evaluation_deserialize(void)
{
	ccs_tree_space_t         tree_space;
	ccs_objective_space_t    ospace;
	ccs_tree_configuration_t configuration;
	ccs_evaluation_t         evaluation_ref, evaluation;
	ccs_datum_t              res, d;
	ccs_result_t             err;
	char                    *buff;
	size_t                   buff_size;
	ccs_map_t                map;
	int                      cmp;

	create_tree_tuning_problem(&tree_space, &ospace);

	err = ccs_tree_space_sample(tree_space, NULL, &configuration);
	assert(err == CCS_RESULT_SUCCESS);

	res = ccs_float(1.5);
	err = ccs_create_evaluation(
		ospace, (ccs_search_configuration_t)configuration,
		CCS_RESULT_SUCCESS, 1, &res, &evaluation_ref);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_map(&map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_object_serialize(
		evaluation_ref, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_SIZE, &buff_size,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);
	buff = (char *)malloc(buff_size);
	assert(buff);
	err = ccs_object_serialize(
		evaluation_ref, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_SERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&evaluation, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_HANDLE);

	d = ccs_object(ospace);
	d.flags |= CCS_DATUM_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(ospace));
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&evaluation, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_ERROR_INVALID_HANDLE);

	d = ccs_object(tree_space);
	d.flags |= CCS_DATUM_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(tree_space));
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_object_deserialize(
		(ccs_object_t *)&evaluation, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_binding_cmp(
		(ccs_binding_t)evaluation_ref, (ccs_binding_t)evaluation, &cmp);
	assert(err == CCS_RESULT_SUCCESS);
	assert(!cmp);

	free(buff);
	err = ccs_release_object(map);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(evaluation_ref);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(evaluation);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(configuration);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(ospace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(tree_space);
	assert(err == CCS_RESULT_SUCCESS);
}

int
main(void)
{
	ccs_init();
	test();
	test_tree_evaluation_deserialize();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
