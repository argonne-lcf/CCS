#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>
#include <math.h>

double d = -2.0;

ccs_hyperparameter_t create_dummy_numerical(const char * name) {
	ccs_hyperparameter_t hyperparameter;
	ccs_result_t         err;
	err = ccs_create_numerical_hyperparameter(name, CCS_NUM_FLOAT,
	                                          CCSF(-5.0), CCSF(5.0),
	                                          CCSF(0.0), CCSF(d),
	                                          &hyperparameter);
	d += 1.0;
	if (d >= 5.0)
		d = -5.0;
	assert( err == CCS_SUCCESS );
	return hyperparameter;
}

ccs_hyperparameter_t create_dummy_categorical(const char * name) {
	ccs_datum_t          possible_values[4];
	ccs_hyperparameter_t hyperparameter;
	ccs_result_t         err;
	possible_values[0] = ccs_int(1);
	possible_values[1] = ccs_float(2.0);
	possible_values[2] = ccs_string("toto");
	possible_values[3] = ccs_none;

	err = ccs_create_categorical_hyperparameter(name, 4, possible_values, 0,
	                                            &hyperparameter);
	assert( err == CCS_SUCCESS );
	return hyperparameter;
}

ccs_hyperparameter_t create_dummy_ordinal(const char * name) {
	ccs_datum_t          possible_values[4];
	ccs_hyperparameter_t hyperparameter;
	ccs_result_t         err;
	possible_values[0] = ccs_int(1);
	possible_values[1] = ccs_float(2.0);
	possible_values[2] = ccs_string("toto");
	possible_values[3] = ccs_none;

	err = ccs_create_ordinal_hyperparameter(name, 4, possible_values, 0,
	                                        &hyperparameter);
	assert( err == CCS_SUCCESS );
	return hyperparameter;
}

void test_expression_wrapper(ccs_expression_type_t type,
                             size_t count,
                             ccs_datum_t *nodes,
                             ccs_configuration_space_t context,
                             ccs_datum_t *inputs,
                             ccs_datum_t eres,
                             ccs_result_t eerr) {
	ccs_result_t     err;
	ccs_expression_t expression;
	ccs_datum_t      result;

	err = ccs_create_expression(type, count, nodes, &expression);
	assert( err == CCS_SUCCESS );
	err = ccs_expression_eval(expression, (ccs_context_t)context, inputs, &result);
	assert( err == eerr );
	if (eerr != CCS_SUCCESS) {
		err = ccs_release_object(expression);
		assert( err == CCS_SUCCESS );
		return;
	}
	assert( result.type == eres.type );
	switch (result.type) {
	case CCS_INTEGER:
	case CCS_BOOLEAN:
		assert( result.value.i == eres.value.i );
		break;
	case CCS_FLOAT:
		assert( result.value.f == eres.value.f );
		break;
	default:
		assert( 0 );

	}
	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );
}

void test_equal_literal() {
	ccs_datum_t               nodes[2];

	nodes[0] = ccs_float(1.0);
	nodes[1] = ccs_float(1.0);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, NULL, NULL, ccs_bool(CCS_TRUE), CCS_SUCCESS);

	nodes[0] = ccs_float(0.0);
	nodes[1] = ccs_float(1.0);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, NULL, NULL, ccs_bool(CCS_FALSE), CCS_SUCCESS);

	nodes[0] = ccs_int(1);
	nodes[1] = ccs_float(1.0);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, NULL, NULL, ccs_bool(CCS_TRUE), CCS_SUCCESS);

	nodes[0] = ccs_float(0.0);
	nodes[1] = ccs_int(1);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, NULL, NULL, ccs_bool(CCS_FALSE), CCS_SUCCESS);

	nodes[0] = ccs_none;
	nodes[1] = ccs_none;
	test_expression_wrapper(CCS_EQUAL, 2, nodes, NULL, NULL, ccs_bool(CCS_TRUE), CCS_SUCCESS);

	nodes[0] = ccs_none;
	nodes[1] = ccs_int(1);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, NULL, NULL, ccs_bool(CCS_FALSE), -CCS_INVALID_VALUE);

	nodes[0] = ccs_string("toto");
	nodes[1] = ccs_string("toto");
	test_expression_wrapper(CCS_EQUAL, 2, nodes, NULL, NULL, ccs_bool(CCS_TRUE), CCS_SUCCESS);

	nodes[0] = ccs_string("tata");
	nodes[1] = ccs_string("toto");
	test_expression_wrapper(CCS_EQUAL, 2, nodes, NULL, NULL, ccs_bool(CCS_FALSE),CCS_SUCCESS);

	nodes[0] = ccs_string("tata");
	nodes[1] = ccs_int(1);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, NULL, NULL, ccs_bool(CCS_FALSE), -CCS_INVALID_VALUE);
}

void test_equal_numerical() {
	ccs_configuration_space_t configuration_space;
	ccs_hyperparameter_t      hyperparameters[2];
	ccs_datum_t               nodes[2];
	ccs_datum_t               values[2];
	ccs_result_t              err;

	err = ccs_create_configuration_space("my_config_space",
	                                     &configuration_space);
	assert( err == CCS_SUCCESS );
	
	hyperparameters[0] = create_dummy_numerical("param1");
	hyperparameters[1] = create_dummy_numerical("param2");

	err = ccs_configuration_space_add_hyperparameters(configuration_space, 2,
	                                                  hyperparameters, NULL);
	assert( err == CCS_SUCCESS );

	nodes[0] = ccs_object(hyperparameters[0]);
	nodes[1] = ccs_float(1.0);
	values[0] = ccs_float(1.0);
	values[1] = ccs_float(0.0);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, configuration_space, values, ccs_bool(CCS_TRUE), CCS_SUCCESS);
	values[0] = ccs_float(0.0);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, configuration_space, values, ccs_bool(CCS_FALSE), CCS_SUCCESS);
	nodes[0] = ccs_float(1.0);
	nodes[1] = ccs_object(hyperparameters[1]);
	values[0] = ccs_float(1.0);
	values[1] = ccs_float(1.0);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, configuration_space, values, ccs_bool(CCS_TRUE), CCS_SUCCESS);
	values[1] = ccs_float(0.0);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, configuration_space, values, ccs_bool(CCS_FALSE), CCS_SUCCESS);
	nodes[0] = ccs_int(0);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, configuration_space, values, ccs_bool(CCS_TRUE), CCS_SUCCESS);
	nodes[0] = ccs_bool(CCS_FALSE);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, configuration_space, values, ccs_bool(CCS_TRUE), -CCS_INVALID_VALUE);

	for (size_t i = 0; i < 2; i++) {
		err = ccs_release_object(hyperparameters[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(configuration_space);
	assert( err == CCS_SUCCESS );
}

void test_equal_categorical() {
	ccs_configuration_space_t configuration_space;
	ccs_hyperparameter_t      hyperparameters[2];
	ccs_datum_t               nodes[2];
	ccs_datum_t               values[2];
	ccs_result_t              err;

	err = ccs_create_configuration_space("my_config_space",
	                                     &configuration_space);
	assert( err == CCS_SUCCESS );
	hyperparameters[0] = create_dummy_categorical("param1");
	hyperparameters[1] = create_dummy_categorical("param2");
	err = ccs_configuration_space_add_hyperparameters(configuration_space, 2,
	                                                  hyperparameters, NULL);
	assert( err == CCS_SUCCESS );

	nodes[0] = ccs_object(hyperparameters[0]);
	nodes[1] = ccs_float(2.0);
	values[0] = ccs_float(2.0);
	values[1] = ccs_int(1);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, configuration_space, values, ccs_bool(CCS_TRUE), CCS_SUCCESS);
	// Values tested must exist in the set
	nodes[1] = ccs_float(3.0);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, configuration_space, values, ccs_bool(CCS_TRUE), -CCS_INVALID_VALUE);
	nodes[1] = ccs_int(1);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, configuration_space, values, ccs_bool(CCS_FALSE), CCS_SUCCESS);

	for (size_t i = 0; i < 2; i++) {
		err = ccs_release_object(hyperparameters[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(configuration_space);
	assert( err == CCS_SUCCESS );
}

void test_equal_ordinal() {
	ccs_configuration_space_t configuration_space;
	ccs_hyperparameter_t      hyperparameters[2];
	ccs_datum_t               nodes[2];
	ccs_datum_t               values[2];
	ccs_result_t              err;

	err = ccs_create_configuration_space("my_config_space",
	                                     &configuration_space);
	assert( err == CCS_SUCCESS );
	hyperparameters[0] = create_dummy_ordinal("param1");
	hyperparameters[1] = create_dummy_ordinal("param2");
	err = ccs_configuration_space_add_hyperparameters(configuration_space, 2,
	                                                  hyperparameters, NULL);
	assert( err == CCS_SUCCESS );

	nodes[0] = ccs_object(hyperparameters[0]);
	nodes[1] = ccs_float(2.0);
	values[0] = ccs_float(2.0);
	values[1] = ccs_int(1);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, configuration_space, values, ccs_bool(CCS_TRUE), CCS_SUCCESS);
	// Values tested must exist in the set
	nodes[1] = ccs_float(3.0);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, configuration_space, values, ccs_bool(CCS_TRUE), -CCS_INVALID_VALUE);
	nodes[1] = ccs_int(1);
	test_expression_wrapper(CCS_EQUAL, 2, nodes, configuration_space, values, ccs_bool(CCS_FALSE), CCS_SUCCESS);

	for (size_t i = 0; i < 2; i++) {
		err = ccs_release_object(hyperparameters[i]);
		assert( err == CCS_SUCCESS );
	}
	err = ccs_release_object(configuration_space);
	assert( err == CCS_SUCCESS );
}

void test_binary_arithmetic(ccs_expression_type_t t,
                            ccs_datum_t a, ccs_datum_t b,
                            ccs_datum_t eres, ccs_result_t eerr) {
	ccs_datum_t nodes[2];
	nodes[0] = a;
	nodes[1] = b;
	test_expression_wrapper(t, 2, nodes, NULL, NULL, eres, eerr);

}

void test_unary_arithmetic(ccs_expression_type_t t,
                           ccs_datum_t a,
                           ccs_datum_t eres, ccs_result_t eerr) {
	test_expression_wrapper(t, 1, &a, NULL, NULL, eres, eerr);
}

void test_arithmetic_add() {
	test_binary_arithmetic(CCS_ADD,
	                       ccs_float(1.0), ccs_float(2.0), ccs_float(1.0+2.0),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_ADD,
	                       ccs_int(1), ccs_float(2.0), ccs_float(1+2.0),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_ADD,
	                       ccs_float(1.0), ccs_int(2), ccs_float(1.0+2),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_ADD,
	                       ccs_int(1), ccs_int(2), ccs_int(1+2),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_ADD,
	                       ccs_int(1), ccs_bool(CCS_TRUE), ccs_none,
	                       -CCS_INVALID_VALUE);
}

void test_arithmetic_substract() {
	test_binary_arithmetic(CCS_SUBSTRACT,
	                       ccs_float(1.0), ccs_float(2.0), ccs_float(1.0-2.0),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_SUBSTRACT,
	                       ccs_int(1), ccs_float(2.0), ccs_float(1-2.0),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_SUBSTRACT,
	                       ccs_float(1.0), ccs_int(2), ccs_float(1.0-2),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_SUBSTRACT,
	                       ccs_int(1), ccs_int(2), ccs_int(1-2),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_SUBSTRACT,
	                       ccs_int(1), ccs_bool(CCS_TRUE), ccs_none,
	                       -CCS_INVALID_VALUE);
}

void test_arithmetic_multiply() {
	test_binary_arithmetic(CCS_MULTIPLY,
	                       ccs_float(3.0), ccs_float(2.0), ccs_float(3.0*2.0),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_MULTIPLY,
	                       ccs_int(3), ccs_float(2.0), ccs_float(3*2.0),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_MULTIPLY,
	                       ccs_float(3.0), ccs_int(2), ccs_float(3.0*2),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_MULTIPLY,
	                       ccs_int(3), ccs_int(2), ccs_int(3*2),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_MULTIPLY,
	                       ccs_int(3), ccs_bool(CCS_TRUE), ccs_none,
	                       -CCS_INVALID_VALUE);
}

void test_arithmetic_divide() {
	test_binary_arithmetic(CCS_DIVIDE,
	                       ccs_float(3.0), ccs_float(2.0), ccs_float(3.0/2.0),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_DIVIDE,
	                       ccs_int(3), ccs_float(2.0), ccs_float(3/2.0),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_DIVIDE,
	                       ccs_float(3.0), ccs_int(2), ccs_float(3.0/2),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_DIVIDE,
	                       ccs_int(3), ccs_int(2), ccs_int(3/2),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_DIVIDE,
	                       ccs_int(3), ccs_bool(CCS_TRUE), ccs_none,
	                       -CCS_INVALID_VALUE);
}

void test_arithmetic_modulo() {
	test_binary_arithmetic(CCS_MODULO,
	                       ccs_float(3.0), ccs_float(2.0), ccs_float(fmod(3.0, 2.0)),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_MODULO,
	                       ccs_int(3), ccs_float(2.0), ccs_float(fmod(3, 2.0)),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_MODULO,
	                       ccs_float(3.0), ccs_int(2), ccs_float(fmod(3.0, 2)),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_MODULO,
	                       ccs_int(3), ccs_int(2), ccs_int(3%2),
	                       CCS_SUCCESS);

	test_binary_arithmetic(CCS_MODULO,
	                       ccs_int(3), ccs_bool(CCS_TRUE), ccs_none,
	                       -CCS_INVALID_VALUE);
}

void test_arithmetic_positive() {
	test_unary_arithmetic(CCS_POSITIVE, ccs_float(3.0),
	                      ccs_float(3.0), CCS_SUCCESS);

	test_unary_arithmetic(CCS_POSITIVE, ccs_int(3),
	                      ccs_int(3), CCS_SUCCESS);

	test_unary_arithmetic(CCS_POSITIVE, ccs_bool(CCS_FALSE),
	                      ccs_none, -CCS_INVALID_VALUE);
}

void test_arithmetic_negative() {
	test_unary_arithmetic(CCS_NEGATIVE, ccs_float(3.0),
	                      ccs_float(-3.0), CCS_SUCCESS);

	test_unary_arithmetic(CCS_NEGATIVE, ccs_int(3),
	                      ccs_int(-3), CCS_SUCCESS);

	test_unary_arithmetic(CCS_NEGATIVE, ccs_bool(CCS_FALSE),
	                      ccs_none, -CCS_INVALID_VALUE);
}

void test_arithmetic_not() {
	test_unary_arithmetic(CCS_NOT, ccs_float(3.0),
	                      ccs_none, -CCS_INVALID_VALUE);

	test_unary_arithmetic(CCS_NOT, ccs_int(3),
	                      ccs_none, -CCS_INVALID_VALUE);

	test_unary_arithmetic(CCS_NOT, ccs_bool(CCS_FALSE),
	                      ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_unary_arithmetic(CCS_NOT, ccs_bool(CCS_TRUE),
	                      ccs_bool(CCS_FALSE), CCS_SUCCESS);
}

void test_arithmetic_and() {
	test_binary_arithmetic(CCS_AND,
	                       ccs_bool(CCS_FALSE), ccs_bool(CCS_FALSE),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_AND,
	                       ccs_bool(CCS_TRUE), ccs_bool(CCS_FALSE),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_AND,
	                       ccs_bool(CCS_FALSE), ccs_bool(CCS_TRUE),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_AND,
	                       ccs_bool(CCS_TRUE), ccs_bool(CCS_TRUE),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_AND,
	                       ccs_int(1), ccs_bool(CCS_TRUE),
	                       ccs_none, -CCS_INVALID_VALUE);
}

void test_arithmetic_or() {
	test_binary_arithmetic(CCS_OR,
	                       ccs_bool(CCS_FALSE), ccs_bool(CCS_FALSE),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_OR,
	                       ccs_bool(CCS_TRUE), ccs_bool(CCS_FALSE),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_OR,
	                       ccs_bool(CCS_FALSE), ccs_bool(CCS_TRUE),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_OR,
	                       ccs_bool(CCS_TRUE), ccs_bool(CCS_TRUE),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_OR,
	                       ccs_int(1), ccs_bool(CCS_TRUE),
	                       ccs_none, -CCS_INVALID_VALUE);
}

void test_arithmetic_less() {
	test_binary_arithmetic(CCS_LESS,
	                       ccs_float(1.0), ccs_float(2.0),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS,
	                       ccs_float(1.0), ccs_float(1.0),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS,
	                       ccs_float(2.0), ccs_float(1.0),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS,
	                       ccs_int(1), ccs_float(2.0),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS,
	                       ccs_float(2.0), ccs_int(1),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS,
	                       ccs_int(1), ccs_int(1),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS,
	                       ccs_bool(CCS_TRUE), ccs_int(1),
	                       ccs_none, -CCS_INVALID_VALUE);

	test_binary_arithmetic(CCS_LESS,
	                       ccs_string("bar"), ccs_string("foo"),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS,
	                       ccs_string("baz"), ccs_string("bar"),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS,
	                       ccs_string("bar"), ccs_string("bar"),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);
}

void test_arithmetic_greater() {
	test_binary_arithmetic(CCS_GREATER,
	                       ccs_float(1.0), ccs_float(2.0),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER,
	                       ccs_float(1.0), ccs_float(1.0),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER,
	                       ccs_float(2.0), ccs_float(1.0),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER,
	                       ccs_int(1), ccs_float(2.0),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER,
	                       ccs_float(2.0), ccs_int(1),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER,
	                       ccs_int(1), ccs_int(1),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER,
	                       ccs_bool(CCS_TRUE), ccs_int(1),
	                       ccs_none, -CCS_INVALID_VALUE);

	test_binary_arithmetic(CCS_GREATER,
	                       ccs_string("bar"), ccs_string("foo"),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER,
	                       ccs_string("baz"), ccs_string("bar"),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER,
	                       ccs_string("bar"), ccs_string("bar"),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);
}

void test_arithmetic_less_or_equal() {
	test_binary_arithmetic(CCS_LESS_OR_EQUAL,
	                       ccs_float(1.0), ccs_float(2.0),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS_OR_EQUAL,
	                       ccs_float(1.0), ccs_float(1.0),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS_OR_EQUAL,
	                       ccs_float(2.0), ccs_float(1.0),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS_OR_EQUAL,
	                       ccs_int(1), ccs_float(2.0),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS_OR_EQUAL,
	                       ccs_float(2.0), ccs_int(1),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS_OR_EQUAL,
	                       ccs_int(1), ccs_int(1),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS_OR_EQUAL,
	                       ccs_bool(CCS_TRUE), ccs_int(1),
	                       ccs_none, -CCS_INVALID_VALUE);

	test_binary_arithmetic(CCS_LESS_OR_EQUAL,
	                       ccs_string("bar"), ccs_string("foo"),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS_OR_EQUAL,
	                       ccs_string("baz"), ccs_string("bar"),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_LESS_OR_EQUAL,
	                       ccs_string("bar"), ccs_string("bar"),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);
}

void test_arithmetic_greater_or_equal() {
	test_binary_arithmetic(CCS_GREATER_OR_EQUAL,
	                       ccs_float(1.0), ccs_float(2.0),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER_OR_EQUAL,
	                       ccs_float(1.0), ccs_float(1.0),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER_OR_EQUAL,
	                       ccs_float(2.0), ccs_float(1.0),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER_OR_EQUAL,
	                       ccs_int(1), ccs_float(2.0),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER_OR_EQUAL,
	                       ccs_float(2.0), ccs_int(1),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER_OR_EQUAL,
	                       ccs_int(1), ccs_int(1),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER_OR_EQUAL,
	                       ccs_bool(CCS_TRUE), ccs_int(1),
	                       ccs_none, -CCS_INVALID_VALUE);

	test_binary_arithmetic(CCS_GREATER_OR_EQUAL,
	                       ccs_string("bar"), ccs_string("foo"),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER_OR_EQUAL,
	                       ccs_string("baz"), ccs_string("bar"),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_GREATER_OR_EQUAL,
	                       ccs_string("bar"), ccs_string("bar"),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);
}

void test_in() {
	ccs_expression_t list;
	ccs_datum_t      values[4];
	ccs_result_t     err;

	values[0] = ccs_float(3.0);
	values[1] = ccs_int(1);
	values[2] = ccs_string("foo");
	values[3] = ccs_bool(CCS_TRUE);

	err = ccs_create_expression(CCS_LIST, 4, values, &list);
	assert( err == CCS_SUCCESS );

	test_binary_arithmetic(CCS_IN,
	                       ccs_float(3.0), ccs_object(list),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_IN,
	                       ccs_int(1), ccs_object(list),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_IN,
	                       ccs_string("foo"), ccs_object(list),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_IN,
	                       ccs_bool(CCS_TRUE), ccs_object(list),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_IN,
	                       ccs_int(3), ccs_object(list),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_IN,
	                       ccs_float(1.0), ccs_object(list),
	                       ccs_bool(CCS_TRUE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_IN,
	                       ccs_float(2.0), ccs_object(list),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_IN,
	                       ccs_int(2), ccs_object(list),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_IN,
	                       ccs_string("bar"), ccs_object(list),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	test_binary_arithmetic(CCS_IN,
	                       ccs_bool(CCS_FALSE), ccs_object(list),
	                       ccs_bool(CCS_FALSE), CCS_SUCCESS);

	err = ccs_release_object(list);
	assert( err == CCS_SUCCESS );
}

void
test_compound() {
	ccs_expression_t      expression1, expression2;
	ccs_datum_t           result;
	ccs_result_t          err;
	ccs_expression_t      nodes[3];
	size_t                num_nodes_ret;
	ccs_expression_type_t type;

	err = ccs_create_binary_expression(CCS_ADD, ccs_float(3.0), ccs_int(1),
	                                   &expression1);
	assert( err == CCS_SUCCESS );
	err = ccs_expression_get_nodes(expression1, 3, nodes, &num_nodes_ret);
	assert( err == CCS_SUCCESS );
	assert( num_nodes_ret == 2 );
	err = ccs_expression_get_type(nodes[0], &type);
	assert( err == CCS_SUCCESS );
	assert( type == CCS_LITERAL );
	err = ccs_literal_get_value(nodes[0], &result);
	assert( err == CCS_SUCCESS );
	assert( result.type == CCS_FLOAT );
	assert( result.value.f == 3.0 );
	err = ccs_expression_get_type(nodes[1], &type);
	assert( err == CCS_SUCCESS );
	assert( type == CCS_LITERAL );
	err = ccs_literal_get_value(nodes[1], &result);
	assert( err == CCS_SUCCESS );
	assert( result.type == CCS_INTEGER );
	assert( result.value.i == 1 );

	err = ccs_create_binary_expression(CCS_MULTIPLY, ccs_float(2.0),
	                                   ccs_object(expression1), &expression2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(expression1);
	assert( err == CCS_SUCCESS );

	err = ccs_expression_eval(expression2, NULL, NULL, &result);
	assert( err == CCS_SUCCESS );
	assert( result.type == CCS_FLOAT );
	assert( result.value.f == 8.0 );
	
	err = ccs_release_object(expression2);
	assert( err == CCS_SUCCESS );
}

void test_get_hyperparameters() {
	ccs_expression_t     expression1, expression2;
	ccs_hyperparameter_t hyperparameter1, hyperparameter2;
	ccs_hyperparameter_t hyperparameters[3];
	ccs_result_t         err;
	size_t               count;

	hyperparameter1 = create_dummy_categorical("param1");
	hyperparameter2 = create_dummy_numerical("param2");

	err = ccs_create_binary_expression(CCS_ADD, ccs_float(3.0), ccs_object(hyperparameter2), &expression1);
	assert( err == CCS_SUCCESS );

	err = ccs_expression_get_hyperparameters(expression1, 0, NULL, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 1 );
	err = ccs_expression_get_hyperparameters(expression1, 3, hyperparameters, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 1 );
	assert( hyperparameters[0] == hyperparameter2 );
	assert( hyperparameters[1] == NULL );
	assert( hyperparameters[2] == NULL );

	err = ccs_create_binary_expression(CCS_EQUAL,
	    ccs_object(hyperparameter1), ccs_object(expression1), &expression2);
	assert( err == CCS_SUCCESS );

	err = ccs_expression_get_hyperparameters(expression2, 0, NULL, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 2 );
	err = ccs_expression_get_hyperparameters(expression2, 3, hyperparameters, NULL);
	assert( err == CCS_SUCCESS );
	assert( hyperparameters[0] != hyperparameters[1] );
	assert( hyperparameters[0] == hyperparameter1 || hyperparameters[0] == hyperparameter2 );
	assert( hyperparameters[1] == hyperparameter1 || hyperparameters[1] == hyperparameter2 );
	assert( hyperparameters[2] == NULL );

	err = ccs_release_object(expression2);
	assert( err == CCS_SUCCESS );

	err = ccs_create_binary_expression(CCS_EQUAL,
	    ccs_object(hyperparameter2), ccs_object(expression1), &expression2);
	assert( err == CCS_SUCCESS );
	err = ccs_expression_get_hyperparameters(expression2, 3, hyperparameters, &count);
	assert( err == CCS_SUCCESS );
	assert( count == 1 );
	assert( hyperparameters[0] == hyperparameter2 );
	assert( hyperparameters[1] == NULL );
	assert( hyperparameters[2] == NULL );
	err = ccs_expression_check_context(expression2, NULL);
	assert( err == -CCS_INVALID_VALUE );

	err = ccs_release_object(hyperparameter1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(expression1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(expression2);
	assert( err == CCS_SUCCESS );
}

void test_check_context() {
	ccs_expression_t          expression1, expression2;
	ccs_hyperparameter_t      hyperparameter1, hyperparameter2, hyperparameter3;
	ccs_configuration_space_t space;
	ccs_result_t              err;

	hyperparameter1 = create_dummy_categorical("param1");
	hyperparameter2 = create_dummy_numerical("param2");
	hyperparameter3 = create_dummy_ordinal("param3");

	err = ccs_create_configuration_space("space", &space);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_add_hyperparameter(space, hyperparameter1, NULL);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_add_hyperparameter(space, hyperparameter2, NULL);
	assert( err == CCS_SUCCESS );

	err = ccs_create_binary_expression(CCS_ADD, ccs_float(3.0), ccs_object(hyperparameter2), &expression1);
	assert( err == CCS_SUCCESS );

	err = ccs_create_binary_expression(CCS_EQUAL,
	    ccs_object(hyperparameter1), ccs_object(expression1), &expression2);
	assert( err == CCS_SUCCESS );

	err = ccs_expression_check_context(expression2, NULL);
	assert( err == -CCS_INVALID_VALUE );
	err = ccs_expression_check_context(expression2, (ccs_context_t)space);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(expression2);
	assert( err == CCS_SUCCESS );

	err = ccs_create_binary_expression(CCS_EQUAL,
	    ccs_object(hyperparameter3), ccs_object(expression1), &expression2);
	assert( err == CCS_SUCCESS );
	err = ccs_expression_check_context(expression2, (ccs_context_t)space);
	assert( err == -CCS_INVALID_HYPERPARAMETER );

	err = ccs_release_object(hyperparameter1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(hyperparameter3);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(expression1);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(expression2);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(space);
	assert( err == CCS_SUCCESS );
}

void test_deserialize_literal() {
	ccs_result_t           err;
	ccs_expression_t       expression;
	ccs_object_type_t      otype;
	ccs_expression_type_t  etype;
	char                  *buff;
	size_t                 buff_size;
	ccs_datum_t            d;

	err = ccs_create_literal(ccs_float(3.0), &expression);
	assert( err == CCS_SUCCESS );

	err = ccs_object_serialize(expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE, &buff_size);
	assert( err == CCS_SUCCESS );
	buff = (char *)malloc(buff_size);
	assert( buff );

	err = ccs_object_serialize(expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_object_get_type(expression, &otype);
	assert( err == CCS_SUCCESS );
	assert( otype == CCS_EXPRESSION );

	err = ccs_expression_get_type(expression, &etype);
	assert( err == CCS_SUCCESS );
	assert( etype == CCS_LITERAL );

	err = ccs_literal_get_value(expression, &d);
	assert( err == CCS_SUCCESS );
	assert( d.type == CCS_FLOAT );
	assert( d.value.f == 3.0 );

	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );
	free(buff);
}

void test_deserialize_variable() {
	ccs_result_t           err;
	ccs_hyperparameter_t   hyperparameter;
	ccs_map_t              handle_map;
	ccs_expression_t       expression;
	ccs_object_type_t      otype;
	ccs_expression_type_t  etype;
	char                  *buff;
	size_t                 buff_size;
	ccs_datum_t            d;

	hyperparameter = create_dummy_numerical("param");

	err = ccs_create_variable(hyperparameter, &expression);
	assert( err == CCS_SUCCESS );

	err = ccs_object_serialize(expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE, &buff_size);
	assert( err == CCS_SUCCESS );
	buff = (char *)malloc(buff_size);
	assert( buff );

	err = ccs_object_serialize(expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff, CCS_DESERIALIZE_OPTION_END);
	assert( err == -CCS_INVALID_OBJECT );

	err = ccs_create_map(&handle_map);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
	                             CCS_DESERIALIZE_OPTION_HANDLE_MAP, handle_map, CCS_DESERIALIZE_OPTION_END);
	assert( err == -CCS_INVALID_HANDLE );

	d = ccs_object(hyperparameter);
	d.flags |= CCS_FLAG_ID;
	err = ccs_map_set(handle_map, d, ccs_object(hyperparameter));
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
	                             CCS_DESERIALIZE_OPTION_HANDLE_MAP, handle_map, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_object_get_type(expression, &otype);
	assert( err == CCS_SUCCESS );
	assert( otype == CCS_EXPRESSION );

	err = ccs_expression_get_type(expression, &etype);
	assert( err == CCS_SUCCESS );
	assert( etype == CCS_VARIABLE );

	err = ccs_variable_get_hyperparameter(expression, &hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(handle_map);
	assert( err == CCS_SUCCESS );
	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );
	free(buff);
}

void test_deserialize() {
	ccs_result_t          err;
	ccs_expression_t      expression;
	ccs_hyperparameter_t  hyperparameter;
	ccs_map_t             handle_map;
	char                 *buff;
	size_t                buff_size;
	ccs_datum_t           d;

	hyperparameter = create_dummy_numerical("param");

	err = ccs_create_binary_expression(CCS_ADD, ccs_float(3.0), ccs_object(hyperparameter), &expression);
	assert( err == CCS_SUCCESS );

	err = ccs_object_serialize(expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_SIZE, &buff_size);
	assert( err == CCS_SUCCESS );
	buff = (char *)malloc(buff_size);
	assert( buff );

	err = ccs_object_serialize(expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
	                             CCS_DESERIALIZE_OPTION_END);
	assert( err == -CCS_INVALID_OBJECT );

	err = ccs_create_map(&handle_map);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
	                             CCS_DESERIALIZE_OPTION_HANDLE_MAP, handle_map, CCS_DESERIALIZE_OPTION_END);
	assert( err == -CCS_INVALID_HANDLE );

	d = ccs_object(hyperparameter);
	d.flags |= CCS_FLAG_ID;
	err = ccs_map_set(handle_map, d, ccs_object(hyperparameter));
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(hyperparameter);
	assert( err == CCS_SUCCESS );

	err = ccs_object_deserialize((ccs_object_t*)&expression, CCS_SERIALIZE_FORMAT_BINARY, CCS_SERIALIZE_OPERATION_MEMORY, buff_size, buff,
	                             CCS_DESERIALIZE_OPTION_HANDLE_MAP, handle_map, CCS_DESERIALIZE_OPTION_END);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(expression);
	assert( err == CCS_SUCCESS );

	err = ccs_release_object(handle_map);
	assert( err == CCS_SUCCESS );
	free(buff);
}

int main() {
	ccs_init();
	test_equal_literal();
	test_equal_numerical();
	test_equal_categorical();
	test_equal_ordinal();
	test_arithmetic_add();
	test_arithmetic_substract();
	test_arithmetic_multiply();
	test_arithmetic_divide();
	test_arithmetic_modulo();
	test_arithmetic_positive();
	test_arithmetic_negative();
	test_arithmetic_not();
	test_arithmetic_and();
	test_arithmetic_or();
	test_arithmetic_less();
	test_arithmetic_greater();
	test_arithmetic_less_or_equal();
	test_arithmetic_greater_or_equal();
	test_compound();
	test_in();
	test_get_hyperparameters();
	test_check_context();
	test_deserialize_literal();
	test_deserialize_variable();
	test_deserialize();
	ccs_fini();
}
