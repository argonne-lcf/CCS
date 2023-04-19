#ifndef _FEATURES_EVALUATION_INTERNAL_H
#define _FEATURES_EVALUATION_INTERNAL_H
#include "binding_internal.h"

struct _ccs_features_evaluation_data_s;
typedef struct _ccs_features_evaluation_data_s _ccs_features_evaluation_data_t;

struct _ccs_features_evaluation_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_error_t (*hash)(
		_ccs_features_evaluation_data_t *data,
		ccs_hash_t                      *hash_ret);

	ccs_error_t (*cmp)(
		_ccs_features_evaluation_data_t *data,
		ccs_features_evaluation_t        other,
		int                             *cmp_ret);
};
typedef struct _ccs_features_evaluation_ops_s _ccs_features_evaluation_ops_t;

struct _ccs_features_evaluation_s {
	_ccs_object_internal_t           obj;
	_ccs_features_evaluation_data_t *data;
};

struct _ccs_features_evaluation_data_s {
	ccs_objective_space_t   objective_space;
	size_t                  num_values;
	ccs_datum_t            *values;
	ccs_configuration_t     configuration;
	ccs_features_t          features;
	ccs_evaluation_result_t result;
};

#endif //_FEATURES_EVALUATION_INTERNAL_H
