#ifndef _CCS_TREE_CONFIGURATION_H
#define _CCS_TREE_CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tree_configuration.h
 * A tree configuration is a position on the tree (see tree.h) of a tree
 * space (see tree_space.h).
 */

/**
 * Create a new instance of a tree configuration on a given tree_space.
 * An empty position targets the root of the tree.
 * @param[in] tree_space
 * @param[in] features an optional features to use. If NULL and a feature space
 *                     was provided at \p configuration_space creation, the
 *                     deafult features of the feature space will be used.
 * @param[in] position_size the number of entries in the \p position array
 * @param[in] position an array of indexes defining a location in the tree.
 *                     can be NULL if \p position_size is 0
 * @param[out] configuration_ret a pointer to the variable that will hold the
 *                               newly created tree configuration
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree space is not a valid CCS
 * tree space; or if \p features is not NULL and is not a valid CCS features
 * @return #CCS_RESULT_ERROR_INVALID_FEATURES if features feature space is not
 * the same as the feature space provided at \p tree_space creation.
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configuration_ret is NULL; or
 * if \p position is NULL and \p position_size is greater than 0; or if \p
 * position does not describe a valid position in the tree space
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new tree configuration
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_tree_configuration(
	ccs_tree_space_t          tree_space,
	ccs_features_t            features,
	size_t                    position_size,
	const size_t             *position,
	ccs_tree_configuration_t *configuration_ret);

/**
 * Get the tree space associated to the configuration.
 * @param[in] configuration
 * @param[out] tree_space_ret a pointer to the variable that will contain the
 *                            tree space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration is not a valid
 * CCS configuration
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configuration_space_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_tree_configuration_get_tree_space(
	ccs_tree_configuration_t configuration,
	ccs_tree_space_t        *tree_space_ret);

/**
 * Get the associated features.
 * @param[in] configuration
 * @param[out] features_ret a pointer to the variable that will contain the
 *                          returned features or NULL if none is associated
 *                          to the configuration.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration is not a valid
 * CCS tree configuration
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p features_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_tree_configuration_get_features(
	ccs_tree_configuration_t configuration,
	ccs_features_t          *features_ret);

/**
 * Get the position of the configuration.
 * @param[in] configuration
 * @param[in] position_size the size of the \p position array
 * @param[out] position an array of size \p position_size to hold the returned
 *                      values, or NULL. If the array is too big, extra values
 *                      are set to 0
 * @param[out] position_size_ret a pointer to a variable that will contain the
 *                               number of values that are or would be returned.
 *                               Can be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration is not a valid
 * CCS tree configuration
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p position is NULL and \p
 * position_size is greater than 0; or if \p position is NULL and \p
 * position_size_ret is NULL; or if \p position_size is less than the number of
 * values that would be returned
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_tree_configuration_get_position(
	ccs_tree_configuration_t configuration,
	size_t                   position_size,
	size_t                  *position,
	size_t                  *position_size_ret);

/**
 * Get the values along the path of the configuration.
 * @param[in] configuration
 * @param[in] num_values the size of the \p values array
 * @param[out] values an array of size \p num_values to hold the returned
 *                    values, or NULL. If the array is too big, extra values
 *                    are set to #CCS_DATA_TYPE_NONE
 * @param[out] num_values_ret a pointer to a variable that will contain the
 *                            number of values that are or would be returned.
 *                            Can be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration is not a valid
 * CCS tree configuration
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p values is NULL and \p
 * num_values is greater than 0; or if \p values is NULL and \p num_values_ret
 * is NULL; or if \p num_values is less than the number of values that would be
 * returned
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_tree_configuration_get_values(
	ccs_tree_configuration_t configuration,
	size_t                   num_values,
	ccs_datum_t             *values,
	size_t                  *num_values_ret);

/**
 * Get the node pointed to by the configuration.
 * @param[in] configuration
 * @param[out] node_ret a pointer to a variable that will contain the node to
 *                      be returned
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration is not a valid
 * CCS tree configuration
 * @return #CCS_RESULT_ERROR_INVALID_VALUE \p node_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_tree_configuration_get_node(
	ccs_tree_configuration_t configuration,
	ccs_tree_t              *node_ret);

/**
 * Compute a hash value for the configuration by hashing together the
 * tree space reference, the position size and the position values.
 * @param[in] configuration
 * @param[out] hash_ret the address of the variable that will contain the hash
 *                      value
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration is not a valid
 * CCS tree configuration
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p hash_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_tree_configuration_hash(
	ccs_tree_configuration_t configuration,
	ccs_hash_t              *hash_ret);

/**
 * Define a strict ordering of tree configuration instances. Tree space,
 * size of position and position values are compared.
 * @param[in] configuration the first configuration
 * @param[in] other_configuration the second configuration
 * @param[out] cmp_ret the pointer to the variable that will contain the result
 *                     of the comparison. Will contain -1, 0, or 1 depending on
 *                     if the first configuration is found to be respectively
 *                     lesser than, equal, or greater then the second
 *                     configuration
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration or \p
 * other_configuration are not a valid CCS tree configurations
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_tree_configuration_cmp(
	ccs_tree_configuration_t configuration,
	ccs_tree_configuration_t other_configuration,
	int                     *cmp_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TREE_CONFIGURATION_H
