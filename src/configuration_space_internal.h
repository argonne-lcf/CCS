#ifndef _CONFIGURATION_SPACE_INTERNAL_H
#define _CONFIGURATION_SPACE_INTERNAL_H
#include "utarray.h"
#define HASH_NONFATAL_OOM 1
#include "uthash.h"
#include "utlist.h"
#include "context_internal.h"

struct _ccs_distribution_wrapper_s;
typedef struct _ccs_distribution_wrapper_s _ccs_distribution_wrapper_t;

struct _ccs_hyperparameter_wrapper_s {
	ccs_hyperparameter_t         hyperparameter;
	size_t                       index;
	const char                  *name;
        UT_hash_handle               hh_name;
        UT_hash_handle               hh_handle;
	size_t                       distribution_index;
	_ccs_distribution_wrapper_t *distribution;
	ccs_expression_t             condition;
	UT_array                    *parents;
	UT_array                    *children;
};
typedef struct _ccs_hyperparameter_wrapper_s _ccs_hyperparameter_wrapper_t;

struct _ccs_distribution_wrapper_s {
	ccs_distribution_t           distribution;
	size_t                       dimension;
	size_t                      *hyperparameter_indexes;
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
	const char                    *name;
	void                          *user_data;
	ccs_rng_t                     rng;
	UT_array                      *hyperparameters;
	_ccs_hyperparameter_wrapper_t *name_hash;
	_ccs_hyperparameter_wrapper_t *handle_hash;
	_ccs_distribution_wrapper_t   *distribution_list;
	UT_array                      *forbidden_clauses;
	ccs_bool_t                     graph_ok;
	UT_array                      *sorted_indexes;
};

#endif //_CONFIGURATION_SPACE_INTERNAL_H
