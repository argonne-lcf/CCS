#ifndef _TREE_INTERNAL_H
#define _TREE_INTERNAL_H
#include "distribution_internal.h"

struct _ccs_tree_data_s;
typedef struct _ccs_tree_data_s _ccs_tree_data_t;

struct _ccs_tree_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_tree_ops_s _ccs_tree_ops_t;

struct _ccs_tree_s {
	_ccs_object_internal_t obj;
	_ccs_tree_data_t      *data;
};

struct _ccs_tree_data_s {
	size_t arity;
	ccs_float_t *
		weights; // Storage for children sum_weights * children bias and own weight at weights[arity]
	ccs_tree_t        *children;
	ccs_float_t        bias;
	ccs_datum_t        value;
	// Helper values
	ccs_float_t        sum_weights;
	ccs_distribution_t distribution;
	ccs_tree_t         parent;
	size_t index; // if parent == NULL index contains tree_space handle
};
#endif //_TREE_INTERNAL_H
