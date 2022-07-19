#ifndef _CCS_TREE_H
#define _CCS_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tree.h
 * A CCS tree object represents a node in a tree structure. Nodes have a given
 * arity and are associated a give datum. Nodes can be sampled for the index of
 * one of their children, but can also return themselves. By default children
 * are sampled according to their subtree weight. This allows uniform sampling
 * of the whole tree by recursively sampling selected children, and stopping
 * when a node returns itself. This behavior can be changed by modifying
 * individual nodes weights, as well as biasing subtrees.
 */

/**
 * Create new tree node.
 * @param[in] arity the arity of the node
 * @param[in] value the value associated with the node
 * @param[out] tree_ret a pointer to the variable that will hold
 *                      the newly created tree node.
 * @return #CCS_SUCCESS on success
 * @return #CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                            tree node.
 */
extern ccs_error_t
ccs_create_tree(
	size_t       arity,
	ccs_datum_t  value,
	ccs_tree_t  *tree_ret);

/**
 * Get the value of a tree node.
 * @param[in] tree
 * @param[out] value_ret a pointer to the variable that will contain
 *                       the value associated with the node.
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_VALUE if \p tree_ret is NULL
 */
extern ccs_error_t
ccs_tree_get_value(
	ccs_tree_t   tree,
	ccs_datum_t *value_ret);

/**
 * Get the arity of a tree node.
 * @param[in] tree
 * @param[out] arity_ret a pointer to the variable that will contain
 *                       the arity of the node.
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_VALUE if \p arity_ret is NULL
 */
extern ccs_error_t
ccs_tree_get_arity(
	ccs_tree_t  tree,
	size_t     *arity_ret);

/**
 * Set an unset child in a tree node at the given index.
 * @param[in] tree
 * @param[in] index
 * @param[in] child
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree; or if
 *                             \p child is not a valid CCS tree
 * @return #CCS_OUT_OF_BOUNDS if \p index is greater than \p tree arity
 * @return #CCS_INVALID_VALUE if \p tree already has a child at \p index
 * @return #CCS_INVALID_TREE if \p child is already a child of another node;
 *                           or if child is the root of a tree space
 */
extern ccs_error_t
ccs_tree_set_child(
	ccs_tree_t tree,
	size_t     index,
	ccs_tree_t child);

/**
 * Query a tree node for the child at the given index.
 * @param[in] tree
 * @param[in] index
 * @param[in] child_ret a pointer to the variable that will hold the
 *                      returned child or NULL if the child was undefined
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_OUT_OF_BOUNDS if \p index is greater than \p tree arity
 * @return #CCS_INVALID_VALUE if \p child_ret is NULL
 */
extern ccs_error_t
ccs_tree_get_child(
	ccs_tree_t  tree,
	size_t      index,
	ccs_tree_t *child_ret);

/**
 * Query all the children of a tree node.
 * @param[in] tree
 * @param[in] num_children the size of the \p children array
 * @param[out] children an array of size \p num_children to hold the returned
 *                      values, or NULL. If the array is too big, extra values
 *                      are set to NULL. If children are undefined, NULL is
 *                      returned as well
 * @param[out] num_children_ret a pointer to a variable that will contain the
 *                               number of children that are or would be returned.
 *                               Can be NULL
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_VALUE if \p children is NULL and \p num_children is
 *                            greater than 0; or if \p children is NULL and \p
 *                            num_children_ret is NULL; or if \p children is
 *                            not NULL and \p num_children is less than the
 *                            arity of \p tree
 */
extern ccs_error_t
ccs_tree_get_children(
	ccs_tree_t  tree,
	size_t      num_children,
	ccs_tree_t *children,
	size_t     *num_children_ret);

/**
 * Get the parent node of a tree node.
 * @param[in] tree
 * @param[out] parent_ret a pointer to a variable that will contain the returned
 *                        parent node or NULL if the node has no parent.
 * @param[out] index_ret a pointer to a variable that will contain the index of
 *                        the node in the parent if it has a parent. Can be NULL
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_VALUE if \p parent_ret is NULL
 */
extern ccs_error_t
ccs_tree_get_parent(
	ccs_tree_t  tree,
	ccs_tree_t *parent_ret,
	size_t     *index_ret);

/**
 * Get the position of a node in it's tree.
 * @param[in] tree
 * @param[in] position_size the size of the \p position array
 * @param[out] position an array of size \p position_size to hold the returned
 *                      values, or NULL. If the array is too big, extra values
 *                      are set to 0
 * @param[out] position_size_ret a pointer to a variable that will contain the
 *                               number of values that are or would be returned.
 *                               Can be NULL
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_VALUE if \p position is NULL and \p position_size is
 *                            greater than 0; or if \p position is NULL and \p
 *                            position_size_ret is NULL; or if \p position_size
 *                            is less than the number of values that would be
 *                            returned
 */
extern ccs_error_t
ccs_tree_get_position(
	ccs_tree_t  tree,
	size_t      position_size,
	size_t     *position,
	size_t     *position_size_ret);

/**
 * Get the values along the path leading to a node in it's tree.
 * @param[in] tree
 * @param[in] num_values the size of the \p values array
 * @param[out] values an array of size \p num_values to hold the returned
 *                    values, or NULL. If the array is too big, extra values
 *                    are set to #CCS_NONE
 * @param[out] num_values_ret a pointer to a variable that will contain the
 *                            number of values that are or would be returned.
 *                            Can be NULL
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_VALUE if \p values is NULL and \p num_values is greater
 *                            than 0; or if \p values is NULL and \p
 *                            num_values_ret is NULL; or if \p num_values is
 *                            less than the number of values that would be
 *                            returned
 */
extern ccs_error_t
ccs_tree_get_values(
	ccs_tree_t   tree,
	size_t       num_values,
	ccs_datum_t *values,
	size_t      *num_values_ret);

/**
 * Check if a position can be reached from a tree node.
 * @param[in] tree
 * @param[in] position_size the number of entries in the \p position array
 * @param[in] position an array of indexes defining a location in the tree.
 * @param[out] is_valid_ret a pointer to a variable that will hold the result
 *                          of the check. Result will be CCS_TRUE if the
 *                          configuration is valid. Result will be CCS_FALSE if
 *                          the position does not reference a node of the tree.
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_VALUE if \p is_valid_ret is NULL; or if \p position is
 *                            NULL and \p position_size is greater than 0
 */
extern ccs_error_t
ccs_tree_position_is_valid(
	ccs_tree_t    tree,
	size_t        position_size,
	const size_t *position,
	ccs_bool_t   *is_valid_ret);

/**
 * Get the values along the path to a given position from a tree node.
 * @param[in] tree
 * @param[in] position_size the number of entries in the \p position array
 * @param[in] position an array of indexes defining a location in the tree
 *                     space. can be NULL if \p position_size is 0
 * @param[in] num_values the size of the \p values array
 * @param[out] values an array of size \p num_values to hold the returned
 *                    values, or NULL. If the array is too big, extra values
 *                    are set to #CCS_NONE
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_TREE if the position does not reference node in the tree.
 * @return #CCS_INVALID_VALUE if \p values is NULL; if \p num_values is less
 *                            than \p position_size + 1; or if \p position is
 *                            NULL and \p position_size is greater than 0
 */
extern ccs_error_t
ccs_tree_get_values_at_position(
	ccs_tree_t    tree,
	size_t        position_size,
	const size_t *position,
	size_t        num_values,
	ccs_datum_t  *values);

/**
 * Get the node at a given position from a tree node.
 * @param[in] tree
 * @param[in] position_size the number of entries in the \p position array
 * @param[in] position an array of indexes defining a location in the tree
 *                     space. can be NULL if \p position_size is 0
 * @param[out] tree_ret a pointer to the variable that will contain the node
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_VALUE if \p tree_ret is NULL; or if \p position is
 *                            NULL and \p position_size is greater than 0
 * @return #CCS_INVALID_TREE if the position does not define a valid position
 *                           in the tree space, or if this position is
 *                           undefined in a static tree space.
 */
extern ccs_error_t
ccs_tree_get_node_at_position(
	ccs_tree_t    tree,
	size_t        position_size,
	const size_t *position,
	ccs_tree_t   *tree_ret);

/**
 * Get the weight of a tree node.
 * @param[in] tree
 * @param[out] weight_ret a pointer to the variable that will contain the
 *                        returned weight
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_VALUE if \p weight_ret is NULL
 */
extern ccs_error_t
ccs_tree_get_weight(
	ccs_tree_t   tree,
	ccs_float_t *weight_ret);

/**
 * Set the weight of a tree node.
 * @param[in,out] tree
 * @param[in] weight
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 */
extern ccs_error_t
ccs_tree_set_weight(
	ccs_tree_t  tree,
	ccs_float_t weight);

/**
 * Get the bias of a subtree.
 * @param[in] tree
 * @param[out] bias_ret a pointer to the variable that will contain the
 *                        returned bias
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_VALUE if \p bias_ret is NULL
 */
extern ccs_error_t
ccs_tree_get_bias(
	ccs_tree_t   tree,
	ccs_float_t *bias_ret);

/**
 * Set the bias of a tree node.
 * @param[in,out] tree
 * @param[in] bias
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 */
extern ccs_error_t
ccs_tree_set_bias(
	ccs_tree_t  tree,
	ccs_float_t bias);

/**
 * Sample the child index space of a a tree node.  If the returned index is
 * equal to the arity, the node sampled itself.  Sampling is proportional to
 * each subtree weight and to the tree node own weight.  A subtree weight is
 * computed as the sum of each of it's subtree weights plus it's root node weight,
 * multiplied by it's bias.
 * @param[in] tree
 * @param[in] rng the rng to use while sampling
 * @param[out] index_ret a pointer to the variable that will contain the returned index
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_VALUE if \p index_ret is NULL
 * @return #CCS_INVALID_DISTRIBUTION if all the weights of \p tree and all it's subtrees are 0
 */
extern ccs_error_t
ccs_tree_sample(
	ccs_tree_t          tree,
	ccs_rng_t           rng,
	size_t             *index_ret);

/**
 * Sample the child index space of a a tree node.  If the returned index is
 * equal to the arity, the node sampled itself.  Sampling is proportional to
 * each subtree weight and to the tree node own weight.  A subtree weight is
 * computed as the sum of each of it's subtree weights plus it's root node weight,
 * multiplied by it's bias.
 * @param[in] tree
 * @param[in] rng the rng to use while sampling
 * @param[in] num_indices
 * @param[out] indices an array of \p num_indices which will hold the returned values
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p tree is not a valid CCS tree
 * @return #CCS_INVALID_VALUE if \p indices is NULL and \p num_indices is greater than 0
 * @return #CCS_INVALID_DISTRIBUTION if all the weights of \p tree and all it's subtrees are 0
 */
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
