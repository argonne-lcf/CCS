#ifndef _CONFIGURATION_SPACE_INTERNAL_H
#define _CONFIGURATION_SPACE_INTERNAL_H
#include "context_internal.h"

struct _ccs_distribution_wrapper_s;
typedef struct _ccs_distribution_wrapper_s _ccs_distribution_wrapper_t;

struct _ccs_configuration_space_data_s;
typedef struct _ccs_configuration_space_data_s _ccs_configuration_space_data_t;

struct _ccs_configuration_space_ops_s {
	_ccs_context_ops_t ops;
};
typedef struct _ccs_configuration_space_ops_s _ccs_configuration_space_ops_t;

struct _ccs_configuration_space_s {
	_ccs_object_internal_t           obj;
	_ccs_configuration_space_data_t *data;
};

struct _ccs_configuration_space_data_s {
	const char                  *name;
	size_t                       num_parameters;
	ccs_parameter_t             *parameters;
	_ccs_parameter_index_hash_t *hash_elems;
	_ccs_parameter_index_hash_t *name_hash;
	_ccs_parameter_index_hash_t *handle_hash;
	ccs_expression_t            *conditions;
	UT_array                   **parents;
	UT_array                   **children;
	size_t                      *sorted_indexes;
	size_t                       num_forbidden_clauses;
	ccs_expression_t            *forbidden_clauses;
	ccs_rng_t                    rng;
	ccs_feature_space_t          feature_space;
	ccs_distribution_space_t     default_distribution_space;
};

#endif //_CONFIGURATION_SPACE_INTERNAL_H
