#include "cconfigspace_internal.h"
#include "tuner_internal.h"
#include "features_internal.h"

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
ccs_tuner_get_search_space(
	ccs_tuner_t         tuner,
	ccs_search_space_t *search_space_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	CCS_CHECK_PTR(search_space_ret);
	_ccs_tuner_common_data_t *d = (_ccs_tuner_common_data_t *)tuner->data;
	*search_space_ret           = d->search_space;
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
ccs_tuner_get_feature_space(
	ccs_tuner_t          tuner,
	ccs_feature_space_t *feature_space_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	CCS_CHECK_PTR(feature_space_ret);
	_ccs_tuner_common_data_t *d = (_ccs_tuner_common_data_t *)tuner->data;
	*feature_space_ret          = d->feature_space;
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_tuner_ask(
	ccs_tuner_t                 tuner,
	ccs_features_t              features,
	size_t                      num_configurations,
	ccs_search_configuration_t *configurations,
	size_t                     *num_configurations_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	if (features) {
		_ccs_tuner_common_data_t *d =
			(_ccs_tuner_common_data_t *)tuner->data;
		CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
		CCS_REFUTE(
			features->data->feature_space != d->feature_space,
			CCS_RESULT_ERROR_INVALID_FEATURES);
	}
	CCS_CHECK_ARY(num_configurations, configurations);
	CCS_REFUTE(
		!configurations && !num_configurations_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_result_t      err = CCS_RESULT_SUCCESS;
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	CCS_OBJ_RDLOCK(tuner);
	CCS_VALIDATE_ERR_GOTO(
		err,
		ops->ask(
			tuner, features, num_configurations, configurations,
			num_configurations_ret),
		err_tuner_lock);
err_tuner_lock:
	CCS_OBJ_UNLOCK(tuner);
	return err;
}

ccs_result_t
ccs_tuner_tell(
	ccs_tuner_t       tuner,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	CCS_CHECK_ARY(num_evaluations, evaluations);
	ccs_result_t      err = CCS_RESULT_SUCCESS;
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	CCS_OBJ_WRLOCK(tuner);
	CCS_VALIDATE_ERR_GOTO(
		err, ops->tell(tuner, num_evaluations, evaluations),
		err_tuner_lock);
err_tuner_lock:
	CCS_OBJ_UNLOCK(tuner);
	return err;
}

ccs_result_t
ccs_tuner_get_optima(
	ccs_tuner_t       tuner,
	ccs_features_t    features,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations,
	size_t           *num_evaluations_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	if (features) {
		_ccs_tuner_common_data_t *d =
			(_ccs_tuner_common_data_t *)tuner->data;
		CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
		CCS_REFUTE(
			features->data->feature_space != d->feature_space,
			CCS_RESULT_ERROR_INVALID_FEATURES);
	}
	CCS_CHECK_ARY(num_evaluations, evaluations);
	CCS_REFUTE(
		!evaluations && !num_evaluations_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_result_t      err = CCS_RESULT_SUCCESS;
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	CCS_OBJ_RDLOCK(tuner);
	CCS_VALIDATE_ERR_GOTO(
		err,
		ops->get_optima(
			tuner, features, num_evaluations, evaluations,
			num_evaluations_ret),
		err_tuner_lock);
err_tuner_lock:
	CCS_OBJ_UNLOCK(tuner);
	return err;
}

ccs_result_t
ccs_tuner_get_history(
	ccs_tuner_t       tuner,
	ccs_features_t    features,
	size_t            num_evaluations,
	ccs_evaluation_t *evaluations,
	size_t           *num_evaluations_ret)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	if (features) {
		_ccs_tuner_common_data_t *d =
			(_ccs_tuner_common_data_t *)tuner->data;
		CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
		CCS_REFUTE(
			features->data->feature_space != d->feature_space,
			CCS_RESULT_ERROR_INVALID_FEATURES);
	}
	CCS_CHECK_ARY(num_evaluations, evaluations);
	CCS_REFUTE(
		!evaluations && !num_evaluations_ret,
		CCS_RESULT_ERROR_INVALID_VALUE);
	ccs_result_t      err = CCS_RESULT_SUCCESS;
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	CCS_OBJ_RDLOCK(tuner);
	CCS_VALIDATE_ERR_GOTO(
		err,
		ops->get_history(
			tuner, features, num_evaluations, evaluations,
			num_evaluations_ret),
		err_tuner_lock);
err_tuner_lock:
	CCS_OBJ_UNLOCK(tuner);
	return err;
}

ccs_result_t
ccs_tuner_suggest(
	ccs_tuner_t                 tuner,
	ccs_features_t              features,
	ccs_search_configuration_t *configuration)
{
	CCS_CHECK_OBJ(tuner, CCS_OBJECT_TYPE_TUNER);
	if (features) {
		_ccs_tuner_common_data_t *d =
			(_ccs_tuner_common_data_t *)tuner->data;
		CCS_CHECK_OBJ(features, CCS_OBJECT_TYPE_FEATURES);
		CCS_REFUTE(
			features->data->feature_space != d->feature_space,
			CCS_RESULT_ERROR_INVALID_FEATURES);
	}
	_ccs_tuner_ops_t *ops = ccs_tuner_get_ops(tuner);
	CCS_REFUTE(!ops->suggest, CCS_RESULT_ERROR_UNSUPPORTED_OPERATION);
	CCS_CHECK_PTR(configuration);
	ccs_result_t err = CCS_RESULT_SUCCESS;
	CCS_OBJ_RDLOCK(tuner);
	CCS_VALIDATE_ERR_GOTO(
		err, ops->suggest(tuner, features, configuration),
		err_tuner_lock);
err_tuner_lock:
	CCS_OBJ_UNLOCK(tuner);
	return err;
}
