#include "test_utils.h"
#include <assert.h>

void
print_ccs_error_stack(void)
{
	ccs_error_stack_t       err;
	ccs_result_t            code;
	const char             *msg;
	size_t                  stack_depth;
	ccs_error_stack_elem_t *stack_elems;

	err = ccs_get_thread_error();
	if (!err)
		return;
	ccs_error_stack_get_code(err, &code);
	ccs_get_result_name(code, &msg);
	fprintf(stderr, "CCS Error: %s (%d): ", msg, code);
	ccs_error_stack_get_message(err, &msg);
	fprintf(stderr, "%s\n", msg);
	ccs_error_stack_get_elems(err, 0, NULL, &stack_depth);
	stack_elems = (ccs_error_stack_elem_t *)malloc(
		stack_depth * sizeof(ccs_error_stack_elem_t));
	ccs_error_stack_get_elems(err, stack_depth, stack_elems, NULL);
	for (size_t i = 0; i < stack_depth; i++) {
		fprintf(stderr, "\t%s:%d:%s\n", stack_elems[i].file,
			stack_elems[i].line, stack_elems[i].func);
	}
	free(stack_elems);
	ccs_release_object(err);
}

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

ccs_configuration_space_t
create_2d_plane(void)
{
	ccs_parameter_t           parameters[2];
	ccs_configuration_space_t cspace;
	ccs_result_t              err;

	parameters[0] = create_numerical("x", -5.0, 5.0);
	parameters[1] = create_numerical("y", -5.0, 5.0);

	err           = ccs_create_configuration_space(
                "2dplane", 2, parameters, NULL, 0, NULL, NULL, &cspace);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(parameters[0]);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameters[1]);
	assert(err == CCS_RESULT_SUCCESS);

	return cspace;
}

ccs_objective_space_t
create_height_objective(ccs_configuration_space_t cspace)
{
	ccs_parameter_t       parameter;
	ccs_objective_space_t ospace;
	ccs_expression_t      expression;
	ccs_objective_type_t  otype;
	ccs_result_t          err;

	parameter = create_numerical("z", -CCS_INFINITY, CCS_INFINITY);
	err       = ccs_create_variable(parameter, &expression);
	assert(err == CCS_RESULT_SUCCESS);
	otype = CCS_OBJECTIVE_TYPE_MINIMIZE;
	err   = ccs_create_objective_space(
                "height", (ccs_search_space_t)cspace, 1, &parameter, 1,
                &expression, &otype, &ospace);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_release_object(expression);
	assert(err == CCS_RESULT_SUCCESS);
	err = ccs_release_object(parameter);
	assert(err == CCS_RESULT_SUCCESS);

	return ospace;
}

ccs_feature_space_t
create_knobs(ccs_features_t *features_on, ccs_features_t *features_off)
{
	ccs_parameter_t     knob;
	ccs_feature_space_t fspace;
	ccs_result_t        err;
	ccs_datum_t knobs_values[2] = {ccs_string("on"), ccs_string("off")};

	err                         = ccs_create_categorical_parameter(
                "red knob", 2, knobs_values, 0, &knob);
	assert(err == CCS_RESULT_SUCCESS);

	err = ccs_create_feature_space("knobs", 1, &knob, &fspace);
	assert(err == CCS_RESULT_SUCCESS);
	if (features_on) {
		err = ccs_create_features(fspace, 1, knobs_values, features_on);
		assert(err == CCS_RESULT_SUCCESS);
	}
	if (features_off) {
		err = ccs_create_features(
			fspace, 1, knobs_values + 1, features_off);
		assert(err == CCS_RESULT_SUCCESS);
	}

	err = ccs_release_object(knob);
	assert(err == CCS_RESULT_SUCCESS);

	return fspace;
}
