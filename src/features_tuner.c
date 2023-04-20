#include "cconfigspace_internal.h"
#include "features_tuner_internal.h"

static inline _ccs_features_tuner_ops_t *
ccs_features_tuner_get_ops(ccs_features_tuner_t tuner)
{
	return (_ccs_features_tuner_ops_t *)tuner->obj.ops;
}

ccs_result_t
ccs_features_tuner_get_type(
	ccs_features_tuner_t       tuner,
	ccs_features_tuner_type_t *type_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_FEATURES_TUNER);
	CCS_CHECK_PTR(type_ret);
	_ccs_features_tuner_common_data_t *d =
		(_ccs_features_tuner_common_data_t *)tuner->data;
	*type_ret = d->type;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_tuner_get_name(ccs_features_tuner_t tuner, const char **name_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_FEATURES_TUNER);
	CCS_CHECK_PTR(name_ret);
	_ccs_features_tuner_common_data_t *d =
		(_ccs_features_tuner_common_data_t *)tuner->data;
	*name_ret = d->name;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_tuner_get_configuration_space(
	ccs_features_tuner_t       tuner,
	ccs_configuration_space_t *configuration_space_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_FEATURES_TUNER);
	CCS_CHECK_PTR(configuration_space_ret);
	_ccs_features_tuner_common_data_t *d =
		(_ccs_features_tuner_common_data_t *)tuner->data;
	*configuration_space_ret = d->configuration_space;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_tuner_get_objective_space(
	ccs_features_tuner_t   tuner,
	ccs_objective_space_t *objective_space_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_FEATURES_TUNER);
	CCS_CHECK_PTR(objective_space_ret);
	_ccs_features_tuner_common_data_t *d =
		(_ccs_features_tuner_common_data_t *)tuner->data;
	*objective_space_ret = d->objective_space;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_tuner_get_features_space(
	ccs_features_tuner_t  tuner,
	ccs_features_space_t *features_space_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_FEATURES_TUNER);
	CCS_CHECK_PTR(features_space_ret);
	_ccs_features_tuner_common_data_t *d =
		(_ccs_features_tuner_common_data_t *)tuner->data;
	*features_space_ret = d->features_space;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_tuner_ask(
	ccs_features_tuner_t tuner,
	ccs_features_t       features,
	size_t               num_configurations,
	ccs_configuration_t *configurations,
	size_t              *num_configurations_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_FEATURES_TUNER);
	CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
	CCS_CHECK_ARY(num_configurations, configurations);
	CCS_REFUTE(
		!configurations && !num_configurations_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	_ccs_features_tuner_common_data_t *d =
		(_ccs_features_tuner_common_data_t *)tuner->data;
	ccs_bool_t valid;
	CCS_VALIDATE(ccs_features_space_check_features(
		d->features_space, features, &valid));
	CCS_REFUTE(!valid, CCS_RESULT_ERROR_INVALID_FEATURES);
	_ccs_features_tuner_ops_t *ops = ccs_features_tuner_get_ops(tuner);
	CCS_VALIDATE(ops->ask(
		tuner, features, num_configurations, configurations,
		num_configurations_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_tuner_tell(
	ccs_features_tuner_t       tuner,
	size_t                     num_evaluations,
	ccs_features_evaluation_t *evaluations)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_FEATURES_TUNER);
	CCS_CHECK_ARY(num_evaluations, evaluations);
	_ccs_features_tuner_ops_t *ops = ccs_features_tuner_get_ops(tuner);
	CCS_VALIDATE(ops->tell(tuner, num_evaluations, evaluations));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_tuner_get_optimums(
	ccs_features_tuner_t       tuner,
	ccs_features_t             features,
	size_t                     num_evaluations,
	ccs_features_evaluation_t *evaluations,
	size_t                    *num_evaluations_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_FEATURES_TUNER);
	if (features)
		CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
	CCS_CHECK_ARY(num_evaluations, evaluations);
	CCS_REFUTE(
		!evaluations && !num_evaluations_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	_ccs_features_tuner_ops_t *ops = ccs_features_tuner_get_ops(tuner);
	CCS_VALIDATE(ops->get_optimums(
		tuner, features, num_evaluations, evaluations,
		num_evaluations_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_tuner_get_history(
	ccs_features_tuner_t       tuner,
	ccs_features_t             features,
	size_t                     num_evaluations,
	ccs_features_evaluation_t *evaluations,
	size_t                    *num_evaluations_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_FEATURES_TUNER);
	if (features)
		CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
	CCS_CHECK_ARY(num_evaluations, evaluations);
	CCS_REFUTE(
		!evaluations && !num_evaluations_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	_ccs_features_tuner_ops_t *ops = ccs_features_tuner_get_ops(tuner);
	CCS_VALIDATE(ops->get_history(
		tuner, features, num_evaluations, evaluations,
		num_evaluations_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_features_tuner_suggest(
	ccs_features_tuner_t tuner,
	ccs_features_t       features,
	ccs_configuration_t *configuration)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_FEATURES_TUNER);
	_ccs_features_tuner_ops_t *ops = ccs_features_tuner_get_ops(tuner);
	CCS_REFUTE(!ops->suggest, CCS_RESULT_ERROR_UNSUPPORTED_OPERATION);
	CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
	CCS_CHECK_PTR(configuration);
	_ccs_features_tuner_common_data_t *d =
		(_ccs_features_tuner_common_data_t *)tuner->data;
	ccs_bool_t valid;
	CCS_VALIDATE(ccs_features_space_check_features(
		d->features_space, features, &valid));
	CCS_REFUTE(!valid, CCS_RESULT_ERROR_INVALID_FEATURES);
	CCS_VALIDATE(ops->suggest(tuner, features, configuration));
	return CCS_RESULT_SUCCESS;
}
