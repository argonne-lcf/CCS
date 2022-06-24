#ifndef _CCS_TREE_H
#define _CCS_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

enum ccs_tree_type_e {
	CCS_TREE_TYPE_STATIC,
	CCS_TREE_TYPE_DYNAMIC,
	CCS_TREE_TYPE_MAX,
	CCS_TREE_TYPE_FORCE_32BIT = INT_MAX
};

typedef enum ccs_tree_type_e ccs_tree_type_t;

extern ccs_error_t
ccs_create_static_tree(
	size_t       arity,
	ccs_datum_t  value,
	ccs_tree_t  *tree_ret);

extern ccs_error_t
ccs_create_dynamic_tree(
	size_t       arity,
	ccs_datum_t  value,
	ccs_tree_t  *tree_ret);

extern ccs_error_t
ccs_tree_get_type(
	ccs_tree_t       tree,
	ccs_tree_type_t *type_ret);

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
ccs_tree_set_distribution(
	ccs_tree_t         tree,
	ccs_distribution_t distribution);

extern ccs_error_t
ccs_tree_get_distribution(
	ccs_tree_t          tree,
	ccs_distribution_t *distribution_ret);

extern ccs_error_t
ccs_tree_sample(
	ccs_tree_t          tree,
	ccs_distribution_t  distribution,
	ccs_rng_t           rng,
	size_t             *index_ret);

extern ccs_error_t
ccs_tree_samples(
	ccs_tree_t          tree,
	ccs_distribution_t  distribution,
	ccs_rng_t           rng,
	size_t              num_indices,
	size_t             *indices);

extern ccs_error_t
ccs_static_tree_get_children(
	ccs_tree_t  tree,
	size_t      num_children,
	ccs_tree_t *children,
	size_t     *num_children_ret);

extern ccs_error_t
ccs_dynamic_tree_get_children(
	ccs_tree_t  tree,
	size_t      num_children,
	size_t     *indices,
	ccs_tree_t *children,
	size_t     *num_children_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TREE_SPACE_H
