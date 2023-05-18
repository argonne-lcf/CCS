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
	size_t       arity;
	ccs_float_t *weights; // Storage for children sum_weights * children
			      // bias and own weight at weights[arity]
	ccs_float_t *areas; // Storage for roulette sampling size arity+2
	ccs_tree_t  *children;
	ccs_float_t  bias;
	ccs_datum_t  value;
	ccs_float_t  sum_weights;
	ccs_tree_t   parent;
	size_t index; // if parent == NULL index contains tree_space handle
};

static inline ccs_result_t
_ccs_tree_samples(
	_ccs_tree_data_t *data,
	ccs_rng_t         rng,
	size_t            num_indices,
	size_t           *indices)
{
	CCS_REFUTE(
		data->sum_weights == 0.0,
		CCS_RESULT_ERROR_INVALID_DISTRIBUTION);
	gsl_rng *grng;
	CCS_VALIDATE(ccs_rng_get_gsl_rng(rng, &grng));
	for (size_t i = 0; i < num_indices; i++) {
		ccs_float_t rnd   = gsl_rng_uniform(grng);
		ccs_int_t   index = _ccs_dichotomic_search(
                        data->arity + 1, data->areas, rnd);
		indices[i] = (size_t)index;
	}
	return CCS_RESULT_SUCCESS;
}

#endif //_TREE_INTERNAL_H
