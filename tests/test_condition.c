#include <stdlib.h>
#include <assert.h>
#include <cconfigspace.h>
#include <string.h>
#include <math.h>

ccs_hyperparameter_t create_numerical(const char * name) {
	ccs_hyperparameter_t hyperparameter;
	ccs_error_t          err;
	err = ccs_create_numerical_hyperparameter(name, CCS_NUM_FLOAT,
	                                          CCSF(-1.0), CCSF(1.0),
	                                          CCSF(0.0), CCSF(0),
	                                          NULL, &hyperparameter);
	assert( err == CCS_SUCCESS );
	return hyperparameter;
}

void
test_simple() {
	ccs_hyperparameter_t      hyperparameter1, hyperparameter2;
	ccs_configuration_space_t space;
	ccs_expression_t          expression;
	ccs_configuration_t       configuration;
	ccs_datum_t               values[2];
	ccs_configuration_t       configurations[100];
	ccs_error_t               err;

	hyperparameter1 = create_numerical("param1");
	hyperparameter2 = create_numerical("param2");
	err = ccs_create_configuration_space("space", NULL, &space);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_add_hyperparameter(space, hyperparameter1, NULL);
	assert( err == CCS_SUCCESS );
	err = ccs_configuration_space_add_hyperparameter(space, hyperparameter2, NULL);
	assert( err == CCS_SUCCESS );

	err = ccs_create_binary_expression(CCS_LESS, ccs_object(hyperparameter1),
	                                   ccs_float(0.0), &expression);
	assert( err == CCS_SUCCESS );

	err = ccs_configuration_space_set_condition(space, 1, expression);
	assert( err == CCS_SUCCESS );

	for (int i = 0; i < 100; i ++) {
		ccs_float_t f;
		err = ccs_configuration_space_sample(space, &configuration);
		assert( err == CCS_SUCCESS );
		err = ccs_configuration_get_values(configuration, 2, values, NULL);
		assert( err == CCS_SUCCESS );
		assert( values[0].type == CCS_FLOAT );
		f = values[0].value.f;
		assert( f >= -1.0 && f < 1.0 );
		if (f < 0.0)
			assert( values[1].type == CCS_FLOAT );
		else
			assert( values[1].type == CCS_INACTIVE );
		err = ccs_configuration_space_check_configuration(space, configuration);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(configuration);
		assert( err == CCS_SUCCESS );
	}

	err = ccs_configuration_space_samples(space, 100, configurations);
	assert( err == CCS_SUCCESS );

	for (int i = 0; i < 100; i ++) {
		ccs_float_t f;
		err = ccs_configuration_get_values(configurations[i], 2, values, NULL);
		assert( err == CCS_SUCCESS );
		assert( values[0].type == CCS_FLOAT );
		f = values[0].value.f;
		assert( f >= -1.0 && f < 1.0 );
		if (f < 0.0)
			assert( values[1].type == CCS_FLOAT );
		else
			assert( values[1].type == CCS_INACTIVE );
		err = ccs_configuration_space_check_configuration(space, configurations[i]);
		assert( err == CCS_SUCCESS );
		err = ccs_release_object(configurations[i]);
		assert( err == CCS_SUCCESS );
	}

	err = ccs_release_object(expression);
	err = ccs_release_object(hyperparameter1);
	err = ccs_release_object(hyperparameter2);
	err = ccs_release_object(space);
}

int main() {
	ccs_init();
	test_simple();
	return 0;
}
