#ifndef _FEATURES_EVALUATION_INTERNAL_H
#define _FEATURES_EVALUATION_INTERNAL_H

struct _ccs_features_evaluation_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_features_evaluation_ops_s _ccs_features_evaluation_ops_t;

struct _ccs_features_evaluation_data_s;
typedef struct _ccs_features_evaluation_data_s _ccs_features_evaluation_data_t;

struct _ccs_features_evaluation_s {
	_ccs_object_internal_t     obj;
	_ccs_features_evaluation_data_t *data;
};

struct _ccs_features_evaluation_data_s {
	void                  *user_data;
	ccs_objective_space_t  objective_space;
	ccs_configuration_t    configuration;
	ccs_features_t         features;
	ccs_result_t           error;
	size_t                 num_values;
	ccs_datum_t           *values;
};

#endif //_FEATURES_EVALUATION_INTERNAL_H
