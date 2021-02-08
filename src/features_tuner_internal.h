#ifndef _FEATURES_TUNER_INTERNAL_H
#define _FEATURES_TUNER_INTERNAL_H

struct _ccs_features_tuner_data_s;
typedef struct _ccs_features_tuner_data_s _ccs_features_tuner_data_t;

struct _ccs_features_tuner_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*ask)(
		_ccs_features_tuner_data_t *data,
		ccs_features_t              features,
		size_t                      num_configurations,
		ccs_configuration_t        *configurations,
		size_t                     *num_configurations_ret);

	ccs_result_t (*tell)(
		_ccs_features_tuner_data_t *data,
		size_t                      num_evaluations,
		ccs_features_evaluation_t  *evaluations);

	ccs_result_t (*get_optimums)(
		_ccs_features_tuner_data_t *data,
		ccs_features_t              features,
		size_t                      num_evaluations,
		ccs_features_evaluation_t  *evaluations,
		size_t                     *num_evaluations_ret);

	ccs_result_t (*get_history)(
		_ccs_features_tuner_data_t *data,
		ccs_features_t              features,
		size_t                      num_evaluations,
		ccs_features_evaluation_t  *evaluations,
		size_t                     *num_evaluations_ret);

	ccs_result_t (*suggest)(
		_ccs_features_tuner_data_t *data,
		ccs_features_t              features,
		ccs_configuration_t        *configuration);
};
typedef struct _ccs_features_tuner_ops_s _ccs_features_tuner_ops_t;

struct _ccs_features_tuner_s {
	_ccs_object_internal_t      obj;
	_ccs_features_tuner_data_t *data;
};

struct _ccs_features_tuner_common_data_s {
	ccs_features_tuner_type_t  type;
	const char                *name;
	void                      *user_data;
	ccs_configuration_space_t  configuration_space;
	ccs_objective_space_t      objective_space;
	ccs_features_space_t       features_space;
};
typedef struct _ccs_features_tuner_common_data_s _ccs_features_tuner_common_data_t;

#endif //_FEATURES_TUNER_INTERNAL_H
