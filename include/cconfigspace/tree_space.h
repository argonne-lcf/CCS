#ifndef _CCS_TREE_SPACE_H
#define _CCS_TREE_SPACE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tree_space.h
 * A CCS tree space defines an search space over a tree. Tree spaces can be
 * sampled to obtain tree configurations. Configurations con point to unknown
 * children (if they exist). This is where static tree spaces and dynamic tree
 * spaces differ: in a static tree space, the user is responsible for modifying
 * the tree before evaluating these configuration values or node; in a dynamic
 * tree space, when querying a configuration validity, node or values, a
 * callback will be invoked to define the missing children on the configuration
 * path. Sampling a dynamic tree space, by itself, does not modify the tree.
 */

/**
 * CCS supported tree space types.
 */
enum ccs_tree_space_type_e {
	/** A static tree space */
	CCS_TREE_SPACE_TYPE_STATIC,
	/** A dynamic tree space */
	CCS_TREE_SPACE_TYPE_DYNAMIC,
	/** Guard */
	CCS_TREE_SPACE_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_TREE_SPACE_TYPE_FORCE_32BIT = INT_MAX
};

/**
 * A commodity type to represent CCS tree space types.
 */
typedef enum ccs_tree_space_type_e ccs_tree_space_type_t;

/**
 * Create a new static tree space.
 * @param[in] name pointer to a string that will be copied internally
 * @param[in] tree the tree defining the tree space
 * @param[in] rng an optional CCS rng object
 * @param[out] tree_space_ret a pointer to the variable that will hold
 *                            the newly created tree space.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name is NULL; or if \p
 * tree_space_ret is NULL
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p rng is not NULL and is not a
 * valid CCS rng
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new tree space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_static_tree_space(
	const char       *name,
	ccs_tree_t        tree,
	ccs_rng_t         rng,
	ccs_tree_space_t *tree_space_ret);

/**
 * A structure that define the callbacks the user must provide to create a
 * dynamic tree space.
 */
struct ccs_dynamic_tree_space_vector_s {
	/**
	 * The deletion callback that will be called once the reference count
	 * of the tree space reaches 0.
	 */
	ccs_result_t (*del)(ccs_tree_space_t tree_space);

	/**
	 * The all back that will be called when querying a missing children in
	 * a dynamic tree space.
	 * @param[in] tree_space the dynamic tree space
	 * @param[in] parent the parent of the node being queried
	 * @param[in] child_index the index of the child. This value is
	 *                        guaranteed to be less than the arity of the
	 *                        parent node.
	 * @param[out] child_ret a pointer to the variable that will contain
	 *                       the returned child node
	 * @return #CCS_RESULT_SUCCESS on success or an error code describing
	 * the error encountered
	 */
	ccs_result_t (*get_child)(
		ccs_tree_space_t tree_space,
		ccs_tree_t       parent,
		size_t           child_index,
		ccs_tree_t      *child_ret);

	/**
	 * The tree space serialization interface, can be NULL. The tree is
	 * always serialized irrespective of the definition of this callback.
	 */
	ccs_result_t (*serialize_user_state)(
		ccs_tree_space_t tree_space,
		size_t           sate_size,
		void            *state,
		size_t          *state_size_ret);

	/**
	 * The tree space deserialization interface, can be NULL. In this case,
	 * only the tree is deserialized.
	 */
	ccs_result_t (*deserialize_state)(
		ccs_tree_space_t tree_space,
		size_t           state_size,
		const void      *state);
};

/**
 * A commodity type to represent a dynamic tree space callback vector.
 */
typedef struct ccs_dynamic_tree_space_vector_s ccs_dynamic_tree_space_vector_t;

/**
 * Create a new static tree space.
 * @param[in] name pointer to a string that will be copied internally
 * @param[in] tree the tree defining the tree space
 * @param[in] rng an optional CCS rng object
 * @param[in] vector the callback vector implementing the dynamic tree space
 *                   interface
 * @param[in] tree_space_data a pointer to the tree space internal data
 *                            structures can be NULL
 * @param[out] tree_space_ret a pointer to the variable that will hold
 *                            the newly created tree space.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name is NULL; or if \p
 * tree_space_ret is NULL; or if any non optional interface pointer is NULL
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p rng is not NULL and is not a
 * valid CCS rng
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new tree space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_dynamic_tree_space(
	const char                      *name,
	ccs_tree_t                       tree,
	ccs_rng_t                        rng,
	ccs_dynamic_tree_space_vector_t *vector,
	void                            *tree_space_data,
	ccs_tree_space_t                *tree_space_ret);

/**
 * Get the type of a tree space.
 * @param [in] tree_space
 * @param [out] type_ret a pointer to the variable that will contain the
 *                       returned tree space type
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree_space is not a valid CCS
 * tree space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p type_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_tree_space_get_type(
	ccs_tree_space_t       tree_space,
	ccs_tree_space_type_t *type_ret);

/**
 * Get the name of a tree space.
 * @param[in] tree_space
 * @param[out] name_ret a pointer to the variable that will contain a pointer to
 *                      the name of the tree space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name_ret is NULL
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tuner is not a valid CCS tree
 * space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_tree_space_get_name(ccs_tree_space_t tree_space, const char **name_ret);

/**
 * Get the internal rng of the tree space.
 * @param[in] tree_space
 * @param[out] rng_ret a pointer to the variable that will contain the rng
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree_space is not a valid CCS
 * tree space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p rng_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_tree_space_get_rng(ccs_tree_space_t tree_space, ccs_rng_t *rng_ret);

/**
 * Get the tree of a tree space.
 * @param[in] tree_space
 * @param[out] tree_ret a pointer to the variable that will contain the tree
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree_space is not a valid CCS
 * tree space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p rng_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_tree_space_get_tree(ccs_tree_space_t tree_space, ccs_tree_t *tree_ret);

/**
 * Get the node at a given position in a tree space.
 * @param[in] tree_space
 * @param[in] position_size the number of entries in the \p position array
 * @param[in] position an array of indexes defining a location in the tree
 *                     space. can be NULL if \p position_size is 0
 * @param[out] tree_ret a pointer to the variable that will contain the node
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree_space is not a valid CCS
 * tree space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p tree_ret is NULL; or if \p
 * position is NULL and \p position_size is greater than 0
 * @return #CCS_RESULT_ERROR_INVALID_TREE if the position does not define a
 * valid position in the tree space, or if this position is undefined in a
 * static tree space.
 * @remarks
 *   This function is NOT thread-safe for dynamic tree spaces as it can
 *   instanciate new children
 */
extern ccs_result_t
ccs_tree_space_get_node_at_position(
	ccs_tree_space_t tree_space,
	size_t           position_size,
	const size_t    *position,
	ccs_tree_t      *tree_ret);

/**
 * Get the values along the path to a given position in the tree space.
 * @param[in] tree_space
 * @param[in] position_size the number of entries in the \p position array
 * @param[in] position an array of indexes defining a location in the tree
 *                     space. can be NULL if \p position_size is 0
 * @param[in] num_values the size of the \p values array
 * @param[out] values an array of size \p num_values to hold the returned
 *                    values, or NULL. If the array is too big, extra values
 *                    are set to #CCS_DATA_TYPE_NONE
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree_space is not a valid CCS
 * tree space
 * @return #CCS_RESULT_ERROR_INVALID_TREE if the position does not reference a
 * node in the tree.
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p values is NULL; if \p
 * num_values is less than \p position_size + 1; or if \p position is NULL and
 * \p position_size is greater than 0
 * @remarks
 *   This function is NOT thread-safe for dynamic tree spaces as it can
 *   instanciate new children
 */
extern ccs_result_t
ccs_tree_space_get_values_at_position(
	ccs_tree_space_t tree_space,
	size_t           position_size,
	const size_t    *position,
	size_t           num_values,
	ccs_datum_t     *values);

/**
 * Check the validity of a given position in a tree space.
 * @param[in] tree_space
 * @param[in] position_size the number of entries in the \p position array
 * @param[in] position an array of indexes defining a location in the tree
 *                     space. can be NULL if \p position_size is 0
 * @param[out] is_valid_ret a pointer to the variable that will contain the
 *                          result
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree_space is not a valid CCS
 * tree space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p tree_ret is NULL; or if \p
 * position is NULL and \p position_size is greater than 0
 * @return #CCS_RESULT_ERROR_INVALID_TREE if the position does not define a
 * valid position in the tree space, or if this position is undefined in a
 * static tree space.
 * @remarks
 *   This function is NOT thread-safe for dynamic tree spaces as it can
 *   instanciate new children
 */
extern ccs_result_t
ccs_tree_space_check_position(
	ccs_tree_space_t tree_space,
	size_t           position_size,
	const size_t    *position,
	ccs_bool_t      *is_valid_ret);

/**
 * Check the validity of a given configuration in a tree space.
 * @param[in] tree_space
 * @param[in] configuration
 * @param[out] is_valid_ret a pointer to the variable that will contain the
 *                          result
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree_space is not a valid CCS
 * tree space; or if \p configuration is not a valid CCS tree configuration
 * @return #CCS_RESULT_ERROR_INVALID_CONFIGURATION if \p configuration is not
 * associated to \p tree_space
 * @remarks
 *   This function is NOT thread-safe for dynamic tree spaces as it can
 *   instanciate new children
 */
extern ccs_result_t
ccs_tree_space_check_configuration(
	ccs_tree_space_t         tree_space,
	ccs_tree_configuration_t configuration,
	ccs_bool_t              *is_valid_ret);

/**
 * Get a tree configuration sampled randomly from a tree space.  The space is
 * sampled according to the weight and bias of the individual tree nodes. If
 * those are at their default values, the tree nodes are sampled uniformly.
 * @param[in] tree_space
 * @param[in] rng an optional rng to use
 * @param[out] configuration_ret a pointer to the variable that will contain the
 *                               returned tree configuration
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree space is not a valid CCS
 * tree space; or if \p rng is not a valid CCS rng
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if configuration_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate the new configuration
 * @remarks
 *   This function is NOT thread-safe
 */
extern ccs_result_t
ccs_tree_space_sample(
	ccs_tree_space_t          tree_space,
	ccs_rng_t                 rng,
	ccs_tree_configuration_t *configuration_ret);

/**
 * Get a given number of configurations sampled randomly from a tree space.
 * The space is sampled according to the weight and bias of the individual tree
 * nodes. If those are at their default values, the tree nodes are sampled
 * uniformely.
 * @param[in] tree_space
 * @param[in] rng an optional rng to use
 * @param[in] num_configurations the number of requested configurations
 * @param[out] configurations an array of \p num_configurations that will
 *                            contain the requested configurations
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree space is not a valid CCS
 * tree space; or if \p rng is not a valid CCS rng
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configurations is NULL and \p
 * num_configurations is greater than 0
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate the new configurations
 * @remarks
 *   This function is NOT thread-safe
 */
extern ccs_result_t
ccs_tree_space_samples(
	ccs_tree_space_t          tree_space,
	ccs_rng_t                 rng,
	size_t                    num_configurations,
	ccs_tree_configuration_t *configurations);

/**
 * Get the dynamic tree space internal data pointer.
 * @param[in] tree_space
 * @param[out] tree_space_data_ret
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p tree space is not a valid CCS
 * tree space
 * @return #CCS_RESULT_ERROR_INVALID_TREE_SPACE if \p tree space is not a
 * dynamic tree space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p tree_space_data_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_dynamic_tree_space_get_tree_space_data(
	ccs_tree_space_t tree_space,
	void           **tree_space_data_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_TREE_SPACE_H
