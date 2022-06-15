#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

ccs_hyperparameter_t create_numerical(const char * name, double lower, double upper) {
	ccs_hyperparameter_t hyperparameter;
	ccs_result_t         err;
	err = ccs_create_numerical_hyperparameter(name, CCS_NUM_FLOAT,
	                                          CCSF(lower), CCSF(upper),
	                                          CCSF(0.0), CCSF(0),
	                                          &hyperparameter);
	assert( err == CCS_SUCCESS );
	return hyperparameter;
}


void test() {
	ccs_hyperparameter_t      hyperparameter1, hyperparameter2;
	ccs_hyperparameter_t      hyperparameter3;
	ccs_configuration_space_t cspace;
	ccs_objective_space_t     ospace;
	ccs_expression_t          expression;
	ccs_tuner_t               tuner, tuner_copy;
	ccs_result_t              err;
	ccs_datum_t               d;
	char                     *buff;
	size_t                    buff_size;
	ccs_map_t                 map;

	hyperparameter1 = create_numerical("x", -5.0, 5.0);
	hyperparameter2 = create_numerical("y", -5.0, 5.0);

	err = ccs_create_configuration_space("2dplane", &cspace);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_add_hyperparameter(cspace, hyperparameter1, NULL);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_add_hyperparameter(cspace, hyperparameter2, NULL);
	assert( err == CCS_SUCCESS );

	hyperparameter3 = create_numerical("z", -CCS_INFINITY, CCS_INFINITY);
	err = ccs_create_variable(hyperparameter3, &expression);
	assert( err == CCS_SUCCESS );

	err = ccs_create_objective_space("height", &ospace);
	assert( err == CCS_SUCCESS );
	err = ccs_objective_space_add_hyperparameter(ospace, hyperparameter3);
	assert( err == CCS_SUCCESS );
	err = ccs_objective_space_add_objective(ospace, expression, CCS_MINIMIZE);
	assert( err == CCS_SUCCESS );

	err = ccs_create_random_tuner("problem", cspace, ospace, &tuner);
	assert( err == CCS_SUCCESS );

	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t         values[2], res;
		ccs_configuration_t configuration;
		ccs_evaluation_t    evaluation;
		err = ccs_tuner_ask(tuner, 1, &configuration, NULL);
		assert( err == CCS_SUCCESS );
		err = ccs_configuration_get_values(configuration, 2, values, NULL);
		assert( err == CCS_SUCCESS );
		res = ccs_float((values[0].value.f - 1)*(values[0].value.f - 1) +
		                (values[1].value.f - 2)*(values[1].value.f - 2));
		err = ccs_create_evaluation(ospace, configuration, CCS_SUCCESS, 1, &res, &evaluation);
		assert( err == CCS_SUCCESS );
		err = ccs_tuner_tell(tuner, 1, &evaluation);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(configuration);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(evaluation);
		assert( err == CCS_SUCCESS );
	}

	size_t           count;
	ccs_evaluation_t history[100];
	ccs_datum_t      min = ccs_float(INFINITY);
	err = ccs_tuner_get_history(tuner, 100, history, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 100 );
	
	for (size_t i = 0; i < 100; i++) {
		ccs_datum_t      res;
		err = ccs_evaluation_get_objective_value(history[i], 0, &res);
		assert( err == CCS_SUCCESS );
		if (res.value.f < min.value.f)
			min.value.f = res.value.f;
	}

	ccs_evaluation_t evaluation;
	ccs_datum_t      res;
	err = ccs_tuner_get_optimums(tuner, 1, &evaluation, NULL);
	assert( err == CCS_SUCCESS );
	err = ccs_evaluation_get_objective_value(evaluation, 0, &res);
	assert( res.value.f == min.value.f );

	/* Test (de)serialization */
	err = ccs_create_map(&map);
	assert( err == CCS_SUCCESS );
	err = ccs_object_serialize(tuner, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE, &buff_size, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );
	buff = (char *)malloc(buff_size);
	assert( buff );

	err = ccs_object_serialize(tuner, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&tuner_copy, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
	                             CCS_DESERIALIZE_OPTION_HANDLE_MAP, map, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_tuner_get_history(tuner_copy, 100, history, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 100 );

	err = ccs_tuner_get_optimums(tuner_copy, 1, &evaluation, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 1 );

	err = ccs_map_get(map, ccs_object((ccs_object_t)tuner), &d);
	assert( err == CCS_SUCCESS );
	assert( d.type == CCS_OBJECT );
	assert( d.value.o == (ccs_object_t)tuner_copy );

	free(buff);
	err = ccs_release_object(map);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(tuner_copy);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter3);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(cspace);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(ospace);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(tuner);
	assert( err == CCS_SUCCESS );
}

void test_evaluation_deserialize() {
	ccs_hyperparameter_t      hyperparameter1, hyperparameter2;
	ccs_hyperparameter_t      hyperparameter3;
	ccs_configuration_space_t cspace;
	ccs_objective_space_t     ospace;
	ccs_expression_t          expression;
	ccs_result_t              err;
	ccs_configuration_t       configuration;
	ccs_evaluation_t          evaluation_ref, evaluation;
	ccs_datum_t               res, d;
	char                     *buff;
	size_t                    buff_size;
	ccs_map_t                 map;
	int                       cmp;

	hyperparameter1 = create_numerical("x", -5.0, 5.0);
	hyperparameter2 = create_numerical("y", -5.0, 5.0);

	err = ccs_create_configuration_space("2dplane", &cspace);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_add_hyperparameter(cspace, hyperparameter1, NULL);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_add_hyperparameter(cspace, hyperparameter2, NULL);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_sample(cspace, &configuration);
	assert( err == CCS_SUCCESS );

	hyperparameter3 = create_numerical("z", -CCS_INFINITY, CCS_INFINITY);
	err = ccs_create_variable(hyperparameter3, &expression);
	assert( err == CCS_SUCCESS );

	err = ccs_create_objective_space("height", &ospace);
	assert( err == CCS_SUCCESS );
	err = ccs_objective_space_add_hyperparameter(ospace, hyperparameter3);
	assert( err == CCS_SUCCESS );
	err = ccs_objective_space_add_objective(ospace, expression, CCS_MINIMIZE);
	assert( err == CCS_SUCCESS );

	res = ccs_float(1.5);
	err = ccs_create_evaluation(ospace, configuration, CCS_SUCCESS, 1, &res, &evaluation_ref);
	assert( err == CCS_SUCCESS );

	err = ccs_create_map(&map);
	assert( err == CCS_SUCCESS );
	err = ccs_object_serialize(evaluation_ref, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE, &buff_size, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );
	buff = (char *)malloc(buff_size);
	assert( buff );

	err = ccs_object_serialize(evaluation_ref, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_SERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&evaluation, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
	                             CCS_DESERIALIZE_OPTION_HANDLE_MAP, map, CCS_DESERIALIZE_OPTION_END);
	assert( err == -CCS_INVALID_HANDLE );

	d = ccs_object(ospace);
	d.flags |= CCS_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(ospace));
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&evaluation, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
	                             CCS_DESERIALIZE_OPTION_HANDLE_MAP, map, CCS_DESERIALIZE_OPTION_END);
	assert( err == -CCS_INVALID_HANDLE );

	d = ccs_object(cspace);
	d.flags |= CCS_FLAG_ID;
	err = ccs_map_set(map, d, ccs_object(cspace));
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&evaluation, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
	                             CCS_DESERIALIZE_OPTION_HANDLE_MAP, map, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_evaluation_cmp(evaluation_ref, evaluation, &cmp);
	assert( err == CCS_SUCCESS );
	assert( !cmp );

	free(buff);
	err = ccs_release_object(map);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(evaluation_ref);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(evaluation);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(configuration);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter3);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(cspace);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(ospace);
	assert( err == CCS_SUCCESS );
}

int main() {
	ccs_init();
	test();
	test_evaluation_deserialize();
	ccs_clear_thread_error();
	ccs_fini();
	return 0;
}

