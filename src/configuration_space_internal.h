#ifndef _CONFIGURATION_SPACE_INTERNAL_H
#define _CONFIGURATION_SPACE_INTERNAL_H
#include "context_internal.h"

struct _ccs_distribution_wrapper_s;
typedef struct _ccs_distribution_wrapper_s _ccs_distribution_wrapper_t;

struct _ccs_parameter_wrapper_cs_s {
	ccs_parameter_t              parameter;
	size_t                       distribution_index;
	_ccs_distribution_wrapper_t *distribution;
	ccs_expression_t             condition;
	UT_array                    *parents;
	UT_array                    *children;
};
typedef struct _ccs_parameter_wrapper_cs_s _ccs_parameter_wrapper_cs_t;

struct _ccs_distribution_wrapper_s {
	ccs_distribution_t           distribution;
	size_t                       dimension;
	size_t                      *parameter_indexes;
	_ccs_distribution_wrapper_t *prev;
	_ccs_distribution_wrapper_t *next;
};

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
	UT_array                    *parameters;
	_ccs_parameter_index_hash_t *name_hash;
	_ccs_parameter_index_hash_t *handle_hash;
	ccs_rng_t                    rng;
	_ccs_distribution_wrapper_t *distribution_list;
	UT_array                    *forbidden_clauses;
	ccs_bool_t                   graph_ok;
	UT_array                    *sorted_indexes;
};

#endif //_CONFIGURATION_SPACE_INTERNAL_H
