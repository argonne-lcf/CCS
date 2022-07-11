#ifndef _CCS_TREE_H
#define _CCS_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_error_t
ccs_create_tree(
	size_t       arity,
	ccs_datum_t  value,
	ccs_tree_t  *tree_ret);

extern ccs_error_t
ccs_tree_get_value(
	ccs_tree_t   tree,
	ccs_datum_t *value_ret);

extern ccs_error_t
ccs_tree_get_arity(
	ccs_tree_t  tree,
	size_t     *arity_ret);

extern ccs_error_t
ccs_tree_set_child(
	ccs_tree_t tree,
	size_t     index,
	ccs_tree_t child);

extern ccs_error_t
ccs_tree_get_child(
	ccs_tree_t  tree,
	size_t      index,
	ccs_tree_t *child_ret);

extern ccs_error_t
ccs_tree_get_children(
	ccs_tree_t  tree,
	size_t      num_children,
	ccs_tree_t *children,
	size_t     *num_children_ret);

extern ccs_error_t
ccs_tree_get_parent(
	ccs_tree_t  tree,
	ccs_tree_t *parent_ret,
	size_t     *index_ret);

extern ccs_error_t
ccs_tree_get_position(
	ccs_tree_t  tree,
	size_t      position_size,
	size_t     *position,
	size_t     *position_size_ret);

extern ccs_error_t
ccs_tree_get_values(
	ccs_tree_t   tree,
	size_t       num_values,
	ccs_datum_t *values,
	size_t      *num_values_ret);

extern ccs_error_t
ccs_tree_position_is_valid(
	ccs_tree_t    tree,
	size_t        position_size,
	const size_t *position,
	ccs_bool_t   *is_valid_ret);

extern ccs_error_t
ccs_tree_get_values_at_position(
	ccs_tree_t    tree,
	size_t        position_size,
	const size_t *position,
	size_t        num_values,
	ccs_datum_t  *values);

extern ccs_error_t
ccs_tree_get_node_at_position(
	ccs_tree_t    tree,
	size_t        position_size,
	const size_t *position,
	ccs_tree_t   *tree_ret);

extern ccs_error_t
ccs_tree_get_weight(
	ccs_tree_t   tree,
	ccs_float_t *weight_ret);

extern ccs_error_t
ccs_tree_set_weight(
	ccs_tree_t  tree,
	ccs_float_t weight);

extern ccs_error_t
ccs_tree_get_bias(
	ccs_tree_t   tree,
	ccs_float_t *bias_ret);

extern ccs_error_t
ccs_tree_set_bias(
	ccs_tree_t  tree,
	ccs_float_t bias);

// index == arity => sample self
extern ccs_error_t
ccs_tree_sample(
	ccs_tree_t          tree,
	ccs_rng_t           rng,
	size_t             *index_ret);

// index == arity => sample self
extern ccs_error_t
ccs_tree_samples(
	ccs_tree_t          tree,
	ccs_rng_t           rng,
	size_t              num_indices,
	size_t             *indices);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TREE_SPACE_H
