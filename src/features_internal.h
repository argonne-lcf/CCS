#ifndef _FEATURES_INTERNAL_H
#define _FEATURES_INTERNAL_H
#include "binding_internal.h"

struct _ccs_features_data_s;
typedef struct _ccs_features_data_s _ccs_features_data_t;

struct _ccs_features_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*hash)(ccs_features_t features, ccs_hash_t *hash_ret);

	ccs_result_t (*cmp)(
		ccs_features_t features,
		ccs_features_t other,
		int           *cmp_ret);
};
typedef struct _ccs_features_ops_s _ccs_features_ops_t;

struct _ccs_features_s {
	_ccs_object_internal_t obj;
	_ccs_features_data_t  *data;
};

struct _ccs_features_data_s {
	ccs_features_space_t features_space;
	size_t               num_values;
	ccs_datum_t         *values;
};

#endif //_FEATURES_INTERNAL_H
