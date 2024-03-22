#include "cconfigspace_internal.h"
#include "evaluation_binding_internal.h"

static inline _ccs_evaluation_binding_ops_t *
ccs_evaluation_binding_get_ops(ccs_evaluation_binding_t binding)
{
	return (_ccs_evaluation_binding_ops_t *)binding->obj.ops;
}

ccs_result_t
ccs_evaluation_binding_get_objective_space(
	ccs_evaluation_binding_t evaluation,
	ccs_objective_space_t   *objective_space_ret)
{
	CCS_CHECK_EVALUATION_BINDING(evaluation);
	CCS_CHECK_PTR(objective_space_ret);
	*objective_space_ret = evaluation->data->objective_space;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_evaluation_binding_get_result(
	ccs_evaluation_binding_t evaluation,
	ccs_evaluation_result_t *result_ret)
{
	CCS_CHECK_EVALUATION_BINDING(evaluation);
	CCS_CHECK_PTR(result_ret);
	*result_ret = evaluation->data->result;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_evaluation_binding_check(
	ccs_evaluation_binding_t evaluation,
	ccs_bool_t              *is_valid_ret)
{
	CCS_CHECK_EVALUATION_BINDING(evaluation);
	CCS_VALIDATE(ccs_objective_space_check_evaluation(
		evaluation->data->objective_space, evaluation, is_valid_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_evaluation_binding_get_objective_value(
	ccs_evaluation_binding_t evaluation,
	size_t                   index,
	ccs_datum_t             *value_ret)
{
	CCS_CHECK_EVALUATION_BINDING(evaluation);
	CCS_CHECK_PTR(value_ret);
	ccs_expression_t     expression;
	ccs_objective_type_t type;
	CCS_VALIDATE(ccs_objective_space_get_objective(
		evaluation->data->objective_space, index, &expression, &type));
	CCS_VALIDATE(ccs_expression_eval(
		expression, 1, (ccs_binding_t *)&evaluation, value_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_evaluation_binding_get_objective_values(
	ccs_evaluation_binding_t evaluation,
	size_t                   num_values,
	ccs_datum_t             *values,
	size_t                  *num_values_ret)
{
	CCS_CHECK_EVALUATION_BINDING(evaluation);
	CCS_CHECK_ARY(num_values, values);
	CCS_REFUTE(!values && !num_values_ret, CCS_RESULT_ERROR_INVALID_VALUE);
	size_t count;
	CCS_VALIDATE(ccs_objective_space_get_objectives(
		evaluation->data->objective_space, 0, NULL, NULL, &count));
	if (values) {
		CCS_REFUTE(count < num_values, CCS_RESULT_ERROR_INVALID_VALUE);
		for (size_t i = 0; i < count; i++) {
			ccs_expression_t     expression;
			ccs_objective_type_t type;

			CCS_VALIDATE(ccs_objective_space_get_objective(
				evaluation->data->objective_space, i,
				&expression, &type));
			CCS_VALIDATE(ccs_expression_eval(
				expression, 1, (ccs_binding_t *)&evaluation,
				values + i));
		}
		for (size_t i = count; i < num_values; i++)
			values[i] = ccs_none;
	}
	if (num_values_ret)
		*num_values_ret = count;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_evaluation_binding_compare(
	ccs_evaluation_binding_t evaluation,
	ccs_evaluation_binding_t other_evaluation,
	ccs_comparison_t        *result_ret)
{
	CCS_CHECK_EVALUATION_BINDING(evaluation);
	CCS_CHECK_EVALUATION_BINDING(other_evaluation);
	CCS_REFUTE(
		evaluation->obj.type != other_evaluation->obj.type,
		CCS_RESULT_ERROR_INVALID_OBJECT);
	CCS_CHECK_PTR(result_ret);
	if (evaluation == other_evaluation) {
		*result_ret = CCS_COMPARISON_EQUIVALENT;
		return CCS_RESULT_SUCCESS;
	}
	CCS_REFUTE(
		evaluation->data->result || other_evaluation->data->result,
		CCS_RESULT_ERROR_INVALID_OBJECT);
	CCS_REFUTE(
		evaluation->data->objective_space !=
			other_evaluation->data->objective_space,
		CCS_RESULT_ERROR_INVALID_OBJECT);

	_ccs_evaluation_binding_ops_t *ops =
		ccs_evaluation_binding_get_ops(evaluation);
	CCS_VALIDATE(ops->compare(evaluation, other_evaluation, result_ret));
	return CCS_RESULT_SUCCESS;
}
