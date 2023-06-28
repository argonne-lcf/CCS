#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

ccs_parameter_t
create_numerical(const char *name, double lower, double upper)
{
	ccs_parameter_t parameter;
	ccs_result_t    err;
	err = ccs_create_numerical_parameter(
		name, CCS_NUMERIC_TYPE_FLOAT, CCSF(lower), CCSF(upper),
		CCSF(0.0), CCSF(0), &parameter);
	assert(err == CCS_RESULT_SUCCESS);
	return parameter;
}

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
	ccs_parameter_t  parameter;
	ccs_tree_t       root;
	ccs_expression_t expression;
	ccs_result_t     err;

	generate_tree(&root, 5, 0);
	err = ccs_create_static_tree_space("space", root, tree_space);
	assert(err == CCS_RESULT_SUCCESS);

	parameter = create_numerical("sum", -CCS_INFINITY, CCS_INFINITY);
	err       = ccs_create_variable(parameter, &expression);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_objective_space("ospace", 1, &parameter, ospace);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_objective_space_add_objective(
		*ospace, expression, CCS_OBJECTIVE_TYPE_MAXIMIZE);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(root);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(expression);
	assert(err == CCS_RESULT_SUCCESS);
}

struct tuner_last_s {
	ccs_tree_evaluation_t last_eval;
};
typedef struct tuner_last_s tuner_last_t;

ccs_result_t
tuner_last_del(ccs_tree_tuner_t tuner)
{
	tuner_last_t *tuner_data;
	ccs_result_t  err;
	err = ccs_user_defined_tree_tuner_get_tuner_data(
		tuner, (void **)&tuner_data);
	if (err)
		return err;
	if (tuner_data && tuner_data->last_eval)
		ccs_release_object(tuner_data->last_eval);
	free(tuner_data);
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
tuner_last_ask(
	ccs_tree_tuner_t          tuner,
	size_t                    num_configurations,
	ccs_tree_configuration_t *configurations,
	size_t                   *num_configurations_ret)
{
	if (!configurations) {
		*num_configurations_ret = 1;
		return CCS_RESULT_SUCCESS;
	}
	ccs_result_t     err;
	ccs_tree_space_t tree_space;
	err = ccs_tree_tuner_get_tree_space(tuner, &tree_space);
	if (err)
		return err;
	err = ccs_tree_space_samples(
		tree_space, num_configurations, configurations);
	if (err)
		return err;
	if (num_configurations_ret)
		*num_configurations_ret = num_configurations;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
tuner_last_tell(
	ccs_tree_tuner_t       tuner,
	size_t                 num_evaluations,
	ccs_tree_evaluation_t *evaluations)
{
	if (!num_evaluations)
		return CCS_RESULT_SUCCESS;
	ccs_result_t  err;
	tuner_last_t *tuner_data;
	err = ccs_user_defined_tree_tuner_get_tuner_data(
		tuner, (void **)&tuner_data);
	if (err)
		return err;
	err = ccs_retain_object(evaluations[num_evaluations - 1]);
	if (err)
		return err;
	if (tuner_data->last_eval)
		ccs_release_object(tuner_data->last_eval);
	tuner_data->last_eval = evaluations[num_evaluations - 1];
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
tuner_last_get_optima(
	ccs_tree_tuner_t       tuner,
	size_t                 num_evaluations,
	ccs_tree_evaluation_t *evaluations,
	size_t                *num_evaluations_ret)
{
	if (evaluations) {
		if (num_evaluations < 1)
			return CCS_RESULT_ERROR_INVALID_VALUE;
		ccs_result_t  err;
		tuner_last_t *tuner_data;
		err = ccs_user_defined_tree_tuner_get_tuner_data(
			tuner, (void **)&tuner_data);
		if (err)
			return err;
		evaluations[0] = tuner_data->last_eval;
		for (size_t i = 1; i < num_evaluations; i++)
			evaluations[i] = NULL;
	}
	if (num_evaluations_ret)
		*num_evaluations_ret = 1;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
tuner_last_get_history(
	ccs_tree_tuner_t       tuner,
	size_t                 num_evaluations,
	ccs_tree_evaluation_t *evaluations,
	size_t                *num_evaluations_ret)
{
	if (evaluations) {
		if (num_evaluations < 1)
			return CCS_RESULT_ERROR_INVALID_VALUE;
		ccs_result_t  err;
		tuner_last_t *tuner_data;
		err = ccs_user_defined_tree_tuner_get_tuner_data(
			tuner, (void **)&tuner_data);
		if (err)
			return err;
		evaluations[0] = tuner_data->last_eval;
		for (size_t i = 1; i < num_evaluations; i++)
			evaluations[i] = NULL;
	}
	if (num_evaluations_ret)
		*num_evaluations_ret = 1;
	return CCS_RESULT_SUCCESS;
}

ccs_user_defined_tree_tuner_vector_t tuner_last_vector = {
	&tuner_last_del,
	&tuner_last_ask,
	&tuner_last_tell,
	&tuner_last_get_optima,
	&tuner_last_get_history,
	NULL,
	NULL,
	NULL};

void
test()
{
	ccs_tree_space_t      tree_space;
	ccs_objective_space_t ospace;
	ccs_tree_tuner_t      tuner, tuner_copy;
	ccs_result_t          err;
	tuner_last_t         *tuner_data;
	ccs_datum_t           d;
	char                 *buff;
	size_t                buff_size;
	ccs_map_t             map;

	create_tree_tuning_problem(&tree_space, &ospace);

	tuner_data = (tuner_last_t *)calloc(1, sizeof(tuner_last_t));
	assert(tuner_data);

	err = ccs_create_user_defined_tree_tuner(
		"problem", tree_space, ospace, &tuner_last_vector, tuner_data,
		&tuner);
	assert(err == CCS_RESULT_SUCCESS);

	ccs_tree_evaluation_t last_evaluation;
	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t              values[6], res;
		size_t                   num_values;
		ccs_tree_configuration_t configuration;
		ccs_tree_evaluation_t    evaluation;
		ccs_int_t                v;
		err = ccs_tree_tuner_ask(tuner, 1, &configuration, NULL);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_tree_configuration_get_values(
			configuration, 6, values, &num_values);
		assert(err == CCS_RESULT_SUCCESS);
		v = 0;
		for (size_t i = 0; i < num_values; i++) {
			v += values[i].value.i;
		}
		res = ccs_float(v);
		err = ccs_create_tree_evaluation(
			ospace, configuration, CCS_RESULT_SUCCESS, 1, &res,
			&evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_tree_tuner_tell(tuner, 1, &evaluation);
		assert(err == CCS_RESULT_SUCCESS);
		last_evaluation = evaluation;
		err             = ccs_release_object(configuration);
		assert(err == CCS_RESULT_SUCCESS);
		err = ccs_release_object(evaluation);
		assert(err == CCS_RESULT_SUCCESS);
	}

	size_t                count;
	ccs_tree_evaluation_t history[100];
	err = ccs_tree_tuner_get_history(tuner, 100, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 1);

	ccs_tree_evaluation_t evaluation;
	err = ccs_tree_tuner_get_optima(tuner, 1, &evaluation, NULL);
	assert(err == CCS_RESULT_SUCCESS);
	assert(last_evaluation == evaluation);

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

	tuner_data = (tuner_last_t *)calloc(1, sizeof(tuner_last_t));
	assert(tuner_data);

	err = ccs_object_deserialize(
		(ccs_object_t *)&tuner_copy, CCS_SERIALIZE_FORMAT_BINARY,
		CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
		CCS_DESERIALIZE_OPTION_HANDLE_MAP, map,
		CCS_DESERIALIZE_OPTION_VECTOR, &tuner_last_vector,
		CCS_DESERIALIZE_OPTION_DATA, tuner_data,
		CCS_DESERIALIZE_OPTION_END);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_tree_tuner_get_history(tuner_copy, 100, history, &count);
	assert(err == CCS_RESULT_SUCCESS);
	assert(count == 1);

	err = ccs_tree_tuner_get_optima(tuner_copy, 1, &evaluation, &count);
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

int
main()
{
	ccs_init();
	test();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}
