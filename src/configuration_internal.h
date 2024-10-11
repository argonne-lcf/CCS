#ifndef _CONFIGURATION_INTERNAL_H
#define _CONFIGURATION_INTERNAL_H
#include "binding_internal.h"

struct _ccs_configuration_data_s;
typedef struct _ccs_configuration_data_s _ccs_configuration_data_t;

struct _ccs_configuration_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (
		*hash)(ccs_configuration_t configuration, ccs_hash_t *hash_ret);

	ccs_result_t (*cmp)(
		ccs_configuration_t configuration,
		ccs_configuration_t other,
		int                *cmp_ret);
};
typedef struct _ccs_configuration_ops_s _ccs_configuration_ops_t;

struct _ccs_configuration_s {
	_ccs_object_internal_t     obj;
	_ccs_configuration_data_t *data;
};

struct _ccs_configuration_data_s {
	ccs_configuration_space_t configuration_space;
	size_t                    num_values;
	ccs_datum_t              *values;
	ccs_features_t            features;
	size_t                    num_bindings;
	ccs_binding_t             bindings[2];
};

ccs_result_t
_ccs_create_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_features_t            features,
	size_t                    num_values,
	ccs_datum_t              *values,
	ccs_configuration_t      *configuration_ret);

#endif //_CONFIGURATION_INTERNAL_H
