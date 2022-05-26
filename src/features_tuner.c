#include "cconfigspace_internal.h"
#include "features_tuner_internal.h"

static inline _ccs_features_tuner_ops_t *
ccs_features_tuner_get_ops(ccs_features_tuner_t tuner) {
	return (_ccs_features_tuner_ops_t *)tuner->obj.ops;
}

ccs_result_t
ccs_features_tuner_get_type(ccs_features_tuner_t       tuner,
                            ccs_features_tuner_type_t *type_ret) {
	CCS_CHECK_OBJ(tuner, CCS_FEATURES_TUNER);
	CCS_CHECK_PTR(type_ret);
	_ccs_features_tuner_common_data_t *d =
	    (_ccs_features_tuner_common_data_t *)tuner->data;
	*type_ret = d->type;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_tuner_get_name(ccs_features_tuner_t   tuner,
                            const char           **name_ret) {
	CCS_CHECK_OBJ(tuner, CCS_FEATURES_TUNER);
	CCS_CHECK_PTR(name_ret);
	_ccs_features_tuner_common_data_t *d =
	    (_ccs_features_tuner_common_data_t *)tuner->data;
	*name_ret = d->name;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_tuner_get_configuration_space(
		ccs_features_tuner_t       tuner,
		ccs_configuration_space_t *configuration_space_ret) {
	CCS_CHECK_OBJ(tuner, CCS_FEATURES_TUNER);
	CCS_CHECK_PTR(configuration_space_ret);
	_ccs_features_tuner_common_data_t *d =
	    (_ccs_features_tuner_common_data_t *)tuner->data;
	*configuration_space_ret = d->configuration_space;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_tuner_get_objective_space(
		ccs_features_tuner_t   tuner,
		ccs_objective_space_t *objective_space_ret) {
	CCS_CHECK_OBJ(tuner, CCS_FEATURES_TUNER);
	CCS_CHECK_PTR(objective_space_ret);
	_ccs_features_tuner_common_data_t *d =
	    (_ccs_features_tuner_common_data_t *)tuner->data;
	*objective_space_ret = d->objective_space;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_tuner_get_features_space(
		ccs_features_tuner_t  tuner,
		ccs_features_space_t *features_space_ret) {
	CCS_CHECK_OBJ(tuner, CCS_FEATURES_TUNER);
	CCS_CHECK_PTR(features_space_ret);
	_ccs_features_tuner_common_data_t *d =
	    (_ccs_features_tuner_common_data_t *)tuner->data;
	*features_space_ret = d->features_space;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_features_tuner_ask(ccs_features_tuner_t  tuner,
                       ccs_features_t        features,
                       size_t                num_configurations,
                       ccs_configuration_t  *configurations,
                       size_t               *num_configurations_ret) {
	CCS_CHECK_OBJ(tuner, CCS_FEATURES_TUNER);
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_ARY(num_configurations, configurations);
	if (!configurations && !num_configurations_ret)
		return -CCS_INVALID_VALUE;
	/* TODO: check that the provided features are compatible with the
	 * features space */
	_ccs_features_tuner_ops_t *ops = ccs_features_tuner_get_ops(tuner);
	return ops->ask(tuner->data, features, num_configurations, configurations, num_configurations_ret);
}

ccs_result_t
ccs_features_tuner_tell(ccs_features_tuner_t       tuner,
                        size_t                     num_evaluations,
                        ccs_features_evaluation_t *evaluations) {
	CCS_CHECK_OBJ(tuner, CCS_FEATURES_TUNER);
	CCS_CHECK_ARY(num_evaluations, evaluations);
	_ccs_features_tuner_ops_t *ops = ccs_features_tuner_get_ops(tuner);
	return ops->tell(tuner->data, num_evaluations, evaluations);
}

ccs_result_t
ccs_features_tuner_get_optimums(ccs_features_tuner_t       tuner,
                                ccs_features_t             features,
                                size_t                     num_evaluations,
                                ccs_features_evaluation_t *evaluations,
                                size_t                    *num_evaluations_ret) {
	CCS_CHECK_OBJ(tuner, CCS_FEATURES_TUNER);
	if (features)
		CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_ARY(num_evaluations, evaluations);
	if (!evaluations && !num_evaluations_ret)
		return -CCS_INVALID_VALUE;
	/* TODO: check that the provided features are compatible with the
	 * features space */
	_ccs_features_tuner_ops_t *ops = ccs_features_tuner_get_ops(tuner);
	return ops->get_optimums(tuner->data, features, num_evaluations, evaluations, num_evaluations_ret);
}

ccs_result_t
ccs_features_tuner_get_history(ccs_features_tuner_t       tuner,
                               ccs_features_t             features,
                               size_t                     num_evaluations,
                               ccs_features_evaluation_t *evaluations,
                               size_t                    *num_evaluations_ret) {
	CCS_CHECK_OBJ(tuner, CCS_FEATURES_TUNER);
	if (features)
		CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_ARY(num_evaluations, evaluations);
	if (!evaluations && !num_evaluations_ret)
		return -CCS_INVALID_VALUE;
	/* TODO: check that the provided features are compatible with the
	 * features space */
	_ccs_features_tuner_ops_t *ops = ccs_features_tuner_get_ops(tuner);
	return ops->get_history(tuner->data, features, num_evaluations, evaluations, num_evaluations_ret);
}

ccs_result_t
ccs_features_tuner_suggest(ccs_features_tuner_t  tuner,
                           ccs_features_t        features,
                           ccs_configuration_t  *configuration) {
	CCS_CHECK_OBJ(tuner, CCS_FEATURES_TUNER);
	_ccs_features_tuner_ops_t *ops = ccs_features_tuner_get_ops(tuner);
	if (!ops->suggest)
		return -CCS_UNSUPPORTED_OPERATION;
	CCS_CHECK_OBJ(features, CCS_FEATURES);
	CCS_CHECK_PTR(configuration);
	/* TODO: check that the provided features are compatible with the
	 * features space */
	return ops->suggest(tuner->data, features, configuration);
}


