#include "cconfigspace_internal.h"
#include "tuner_internal.h"

static inline _ccs_tuner_ops_t *
ccs_tuner_get_ops(ccs_tuner_t tuner)
{
	return (_ccs_tuner_ops_t *)tuner->obj.ops;
}

ccs_result_t
ccs_tuner_get_type(ccs_tuner_t tuner, ccs_tuner_type_t *type_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	CCS_CHECK_PTR(type_ret);
	_ccs_tuner_common_data_t *d = (_ccs_tuner_common_data_t *)tuner->data;
	*type_ret                   = d->type;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tuner_get_name(ccs_tuner_t tuner, const char **name_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	CCS_CHECK_PTR(name_ret);
	_ccs_tuner_common_data_t *d = (_ccs_tuner_common_data_t *)tuner->data;
	*name_ret                   = d->name;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tuner_get_configuration_space(
	ccs_tuner_t                tuner,
	ccs_configuration_space_t *configuration_space_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	CCS_CHECK_PTR(configuration_space_ret);
	_ccs_tuner_common_data_t *d = (_ccs_tuner_common_data_t *)tuner->data;
	*configuration_space_ret    = d->configuration_space;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tuner_get_objective_space(
	ccs_tuner_t            tuner,
	ccs_objective_space_t *objective_space_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	CCS_CHECK_PTR(objective_space_ret);
	_ccs_tuner_common_data_t *d = (_ccs_tuner_common_data_t *)tuner->data;
	*objective_space_ret        = d->objective_space;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tuner_ask(
	ccs_tuner_t          tuner,
	size_t               num_configurations,
	ccs_configuration_t *configurations,
	size_t              *num_configurations_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	CCS_CHECK_ARY(num_configurations, configurations);
	CCS_REFUTE(
		!configurations && !num_configurations_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	CCS_VALIDATE(ops->ask(
		tuner, num_configurations, configurations,
		num_configurations_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tuner_tell(
	ccs_tuner_t       tuner,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	CCS_CHECK_ARY(num_evaluations, evaluations);
	/* TODO: check that evaluations have the same objective and
	 * configuration sapce than the tuner */
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	CCS_VALIDATE(ops->tell(tuner, num_evaluations, evaluations));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tuner_get_optima(
	ccs_tuner_t       tuner,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations,
	size_t           *num_evaluations_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	CCS_CHECK_ARY(num_evaluations, evaluations);
	CCS_REFUTE(
		!evaluations && !num_evaluations_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	CCS_VALIDATE(ops->get_optima(
		tuner, num_evaluations, evaluations, num_evaluations_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tuner_get_history(
	ccs_tuner_t       tuner,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations,
	size_t           *num_evaluations_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	CCS_CHECK_ARY(num_evaluations, evaluations);
	CCS_REFUTE(
		!evaluations && !num_evaluations_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	CCS_VALIDATE(ops->get_history(
		tuner, num_evaluations, evaluations, num_evaluations_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tuner_suggest(ccs_tuner_t tuner, ccs_configuration_t *configuration)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	CCS_REFUTE(!ops->suggest, CCS_RESULT_ERROR_UNSUPPORTED_OPERATION);
	CCS_CHECK_PTR(configuration);
	CCS_VALIDATE(ops->suggest(tuner, configuration));
	return CCS_RESULT_SUCCESS;
}
