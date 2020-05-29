#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>

static inline ccs_datum_t
ccs_bool(ccs_bool_t v) {
	ccs_datum_t d;
	d.type = CCS_BOOLEAN;
	d.value.i = v;
	return d;
}

static inline ccs_datum_t
ccs_float(ccs_float_t v) {
	ccs_datum_t d;
	d.type = CCS_FLOAT;
	d.value.f = v;
	return d;
}

static inline ccs_datum_t
ccs_int(ccs_int_t v) {
	ccs_datum_t d;
	d.type = CCS_INTEGER;
	d.value.i = v;
	return d;
}

static inline ccs_datum_t
ccs_object(ccs_object_t v) {
	ccs_datum_t d;
	d.type = CCS_OBJECT;
	d.value.o = v;
	return d;
}

static inline ccs_datum_t
ccs_string(const char *v) {
	ccs_datum_t d;
	d.type = CCS_STRING;
	d.value.s = v;
	return d;
}

double d = -2.0;

ccs_hyperparameter_t create_dummy_numerical(const char * name) {
	ccs_hyperparameter_t hyperparameter;
	ccs_error_t          err;
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

ccs_hyperparameter_t create_dummy_categorical(const char * name) {
	ccs_datum_t          possible_values[4];
	ccs_hyperparameter_t hyperparameter;
	ccs_error_t          err;
	possible_values[0] = ccs_int(1);
	possible_values[1] = ccs_float(2.0);
	possible_values[2] = ccs_string("toto");
	possible_values[3] = ccs_none;

	err = ccs_create_categorical_hyperparameter(name, 4, possible_values, 0,
	                                            NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );
	return hyperparameter;
}

ccs_hyperparameter_t create_dummy_ordinal(const char * name) {
	ccs_datum_t          possible_values[4];
	ccs_hyperparameter_t hyperparameter;
	ccs_error_t          err;
	possible_values[0] = ccs_int(1);
	possible_values[1] = ccs_float(2.0);
	possible_values[2] = ccs_string("toto");
	possible_values[3] = ccs_none;

	err = ccs_create_ordinal_hyperparameter(name, 4, possible_values, 0,
	                                        NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );
	return hyperparameter;
}

void test_expression_wrapper(ccs_expression_type_t type,
                             size_t count,
                             ccs_datum_t *nodes,
                             ccs_configuration_space_t context,
                             ccs_datum_t *inputs,
                             ccs_datum_t eres,
                             ccs_error_t eerr) {
	ccs_error_t      err;
	ccs_expression_t expression;
	ccs_datum_t      result;

	err = ccs_create_expression(type, count, nodes, &expression);
	assert( err == CCS_SUCCESS );
	err = ccs_expression_eval(expression, context, inputs, &result);
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
	ccs_error_t               err;

	err = ccs_create_configuration_space("my_config_space", NULL,
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
	ccs_error_t               err;

	err = ccs_create_configuration_space("my_config_space", NULL,
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
	ccs_error_t               err;

	err = ccs_create_configuration_space("my_config_space", NULL,
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

int main(int argc, char *argv[]) {
	ccs_init();
	test_equal_literal();
	test_equal_numerical();
	test_equal_categorical();
	test_equal_ordinal();
}
