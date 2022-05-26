#ifndef _OBJECTIVE_SPACE_INTERNAL_H
#define _OBJECTIVE_SPACE_INTERNAL_H
#include "utlist.h"
#include "context_internal.h"

struct _ccs_objective_s {
	ccs_expression_t     expression;
	ccs_objective_type_t type;
};
typedef struct _ccs_objective_s _ccs_objective_t;

struct _ccs_objective_space_data_s;
typedef struct _ccs_objective_space_data_s _ccs_objective_space_data_t;

struct _ccs_objective_space_ops_s {
	_ccs_context_ops_t ops;
};
typedef struct _ccs_objective_space_ops_s _ccs_objective_space_ops_t;

struct _ccs_objective_space_s {
	_ccs_object_internal_t       obj;
	_ccs_objective_space_data_t *data;
};

struct _ccs_objective_space_data_s {
	const char                       *name;
	UT_array                         *hyperparameters;
	_ccs_hyperparameter_index_hash_t *name_hash;
	_ccs_hyperparameter_index_hash_t *handle_hash;
	UT_array                         *objectives;
};


#endif //_OBJECTIVE_SPACE_INTERNAL_H
