#ifndef _FEATURES_EVALUATION_INTERNAL_H
#define _FEATURES_EVALUATION_INTERNAL_H
#include "binding_internal.h"

struct _ccs_features_evaluation_data_s;
typedef struct _ccs_features_evaluation_data_s _ccs_features_evaluation_data_t;

struct _ccs_features_evaluation_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*hash)(
		ccs_features_evaluation_t features_evaluation,
		ccs_hash_t               *hash_ret);

	ccs_result_t (*cmp)(
		ccs_features_evaluation_t features_evaluation,
		ccs_features_evaluation_t other,
		int                      *cmp_ret);

	ccs_result_t (*compare)(
		ccs_features_evaluation_t evaluation,
		ccs_features_evaluation_t other_evaluation,
		ccs_comparison_t         *result_ret);
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
	ccs_evaluation_result_t result;
	ccs_configuration_t     configuration;
	ccs_features_t          features;
};

#endif //_FEATURES_EVALUATION_INTERNAL_H
