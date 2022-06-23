#ifndef _CCS_FEATURES_SPACE
#define _CCS_FEATURES_SPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file features_space.h
 * A features space is a context (see context.h) defining a set of
 * hyperparameters.
 */

/**
 * Create a new empty features space.
 * @param[in] name pointer to a string that will be copied internally
 * @param[out] features_space_ret a pointer to the variable that will hold
 *                                     the newly created features space
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p name is NULL; or if \p features_space_ret
 *                             is NULL
 * @return #CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             features space
 */
extern ccs_error_t
ccs_create_features_space(const char           *name,
                          ccs_features_space_t *features_space_ret);

/**
 * Get the name of a features space.
 * @param[in] features_space
 * @param[out] name_ret a pointer to a `char *` variable which will contain a
 *                      pointer to the features space name.
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features space is not a valid CCS features
 *                              space
 * @return #CCS_INVALID_VALUE if \p name_ret is NULL
 */
extern ccs_error_t
ccs_features_space_get_name(ccs_features_space_t   features_space,
                            const char           **name_ret);

/**
 * Add a hyperparameter to the features space.
 * @param[in,out] features_space
 * @param[in] hyperparameter the hyperparameter to add to the features
 *                           space
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features_space is not a valid CCS features
 *                              space; or \p hyperparameter is not a valid CCS
 *                              hyperparameter
 * @return #CCS_INVALID_HYPERPARAMETER if \p hyperparameter is already in the
 *                                      features space; or if a hyperparameter
 *                                      with the same name already exists in the
 *                                      features space
 * @return #CCS_OUT_OF_MEMORY if a memory could not be allocated to store
 *                             the additional hyperparameter and associated data
 *                             structures
 */
extern ccs_error_t
ccs_features_space_add_hyperparameter(ccs_features_space_t features_space,
                                      ccs_hyperparameter_t hyperparameter);

/**
 * Add hyperparameters to the features space.
 * @param[in,out] features_space
 * @param[in] num_hyperparameters the number of provided hyperparameters
 * @param[in] hyperparameters an array of \p num_hyperparameters hyperparameters
 *                            to add to the features space
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features_space is not a valid CCS features
 *                              space; or a hyperparameter is not a valid CCS
 *                              hyperparameter
 * @return #CCS_INVALID_VALUE if \p hyperparameters is NULL and \p
 *                             num_hyperparameters is greater than 0
 * @return #CCS_INVALID_HYPERPARAMETER if a hyperparameter is already in the
 *                                      features space; or if a hyperparameter
 *                                      with the same name already exists in the
 *                                      features space
 * @return #CCS_OUT_OF_MEMORY if memory could not be allocated to store
 *                             additional hyperparameters and associated data
 *                             structures
 */
extern ccs_error_t
ccs_features_space_add_hyperparameters(ccs_features_space_t  features_space,
                                       size_t                num_hyperparameters,
                                       ccs_hyperparameter_t *hyperparameters);

/**
 * Get the number of hyperparameters in a features space.
 * @param[in] features_space
 * @param[out] num_hyperparameters_ret a pointer to the variable that will
 *                                     contain the number of hyperparameters in
 *                                     the features space
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features_space is not a valid CCS features
 *                              space
 * @return #CCS_INVALID_VALUE if \p num_hyperparameters_ret is NULL
 */
extern ccs_error_t
ccs_features_space_get_num_hyperparameters(ccs_features_space_t  features_space,
                                           size_t               *num_hyperparameters_ret);

/**
 * Get an hyperparameter in a features space given its index.
 * @param[in] features_space
 * @param[in] index the index of the hyperparameter to retrieve
 * @param[out] hyperparameter_ret a pointer to the variable that will contain
 *                                the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features_space is not a valid CCS features
 *                              space
 * @return #CCS_INVALID_VALUE if \p hyperparameter_ret is NULL
 * @return #CCS_OUT_OF_BOUNDS if \p index is greater than the count of
 *                             hyperparameters in the features space
 */
extern ccs_error_t
ccs_features_space_get_hyperparameter(ccs_features_space_t  features_space,
                                      size_t                index,
                                      ccs_hyperparameter_t *hyperparameter_ret);

/**
 * Get an hyperparameter in a features space given its name.
 * @param[in] features_space
 * @param[in] name the name of the hyperparameter to retrieve
 * @param[out] hyperparameter_ret a pointer to the variable that will contain
 *                                the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features_space is not a valid CCS features
 *                              space
 * @return #CCS_INVALID_VALUE if \p name or \p hyperparameter_ret are NULL
 * @return #CCS_INVALID_NAME if no hyperparameter with such \p name exist in
 *                            the \p features space
 */
extern ccs_error_t
ccs_features_space_get_hyperparameter_by_name(
		ccs_features_space_t  features_space,
		const char *          name,
		ccs_hyperparameter_t *hyperparameter_ret);

/**
 * Get the index of an hyperparameter in the features space given its name.
 * @param[in] features_space
 * @param[in] name the name of the hyperparameter to retrieve the index of
 * @param[out] index_ret a pointer to the variable that will contain the index
 *                       of hyperparameter in the \p features_space
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features_space is not a valid CCS features
 *                              space
 * @return #CCS_INVALID_VALUE if \p name or \p index_ret are NULL
 * @return #CCS_INVALID_NAME if no hyperparameter with such \p name exist in
 *                            the features space
 */
extern ccs_error_t
ccs_features_space_get_hyperparameter_index_by_name(
		ccs_features_space_t  features_space,
		const char           *name,
		size_t               *index_ret);

/**
 * Get the index of an hyperparameter in the features space.
 * @param[in] features_space
 * @param[in] hyperparameter
 * @param[out] index_ret a pointer to the variable which will contain the index
 *                       of the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features_space is not a valid CCS features
 *                              space
 * @return #CCS_INVALID_VALUE if \p index_ret is NULL
 * @return #CCS_INVALID_HYPERPARAMETER if \p features_space does not
 *                                      contain \p hyperparameter
 */
extern ccs_error_t
ccs_features_space_get_hyperparameter_index(
		ccs_features_space_t  features_space,
		ccs_hyperparameter_t  hyperparameter,
		size_t               *index_ret);

/**
 * Get the indices of a set of hyperparameters in a features space.
 * @param[in] features_space
 * @param[in] num_hyperparameters the number of hyperparameters to query the
 *                                index for
 * @param[in] hyperparameters an array of \p num_hyperparameters hyperparameters
 *                            to query the index for
 * @param[out] indexes an array of \p num_hyperparameters indices that will
 *                     contain the values of the hyperparamters indices
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features_space is not a valid CCS features
 *                              space
 * @return #CCS_INVALID_VALUE if \p hyperparameters is NULL and \p
 *                             num_hyperparameters is greater than 0; or if \p
 *                             indexes is NULL and \p num_hyperparameters is
 *                             greater than 0
 * @return #CCS_INVALID_HYPERPARAMETER if at least one of the hyperparameters
 *                                      is not contained in \p features_space
 */
extern ccs_error_t
ccs_features_space_get_hyperparameter_indexes(
		ccs_features_space_t  features_space,
		size_t                num_hyperparameters,
		ccs_hyperparameter_t *hyperparameters,
		size_t               *indexes);

/**
 * Get the hyperparameters in the given features space.
 * @param[in] features_space
 * @param[in] num_hyperparameters is the number of hyperparameters that can be
 *                                added to \p hyperparameters. If \p
 *                                hyperparameters is not NULL \p
 *                                num_hyperparameters must be greater than 0
 * @param[in] hyperparameters an array of \p num_hyperparameters that will
 *                            contain the returned hyperparameters or NULL. If
 *                            the array is too big, extra values are set to NULL
 * @param[out] num_hyperparameters_ret a pointer to a variable that will contain
 *                                     the number of hyperparameters that are or
 *                                     would be returned. Can be NULL
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features_space is not a valid CCS features
 *                              space
 * @return #CCS_INVALID_VALUE if \p hyperparameters is NULL and \p
 *                             num_hyperparameters is greater than 0; or if \p
 *                             hyperparameters is NULL and
 *                             num_hyperparameters_ret is NULL; or if \p
 *                             num_hyperparameters is less than the number of
 *                             hyperparameters that would be returned
 */
extern ccs_error_t
ccs_features_space_get_hyperparameters(ccs_features_space_t  features_space,
                                       size_t                num_hyperparameters,
                                       ccs_hyperparameter_t *hyperparameters,
                                       size_t               *num_hyperparameters_ret);

/**
 * Validate that a given value at the given index is valid in a features
 * space, and return a sanitized value.
 * @param[in] features_space
 * @param[in] index the index of the value in the features_space
 * @param[in] value the datum to validate
 * @param[out] value_ret a pointer that will contain the validated value. If \p
 *                       value is a string \p value_ret will contain a non
 *                       transient string.
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features_space is not a valid CCS features
 *                              space
 * @return #CCS_OUT_OF_BOUNDS if index is greater than the number of
 *                             hyperparameters in \p features_space
 * @return #CCS_OUT_OF_MEMORY if there was a lack of memory while memoizing a
 *                             string
 * @return #CCS_INVALID_VALUE if the value did not validate or if value_ret is
 *                             NULL
 */
extern ccs_error_t
ccs_features_space_validate_value(ccs_features_space_t  features_space,
                                  size_t                index,
                                  ccs_datum_t           value,
                                  ccs_datum_t          *value_ret);

/**
 * Check that a features is a valid in a features space.
 * @param[in] features_space
 * @param[in] features
 * @param[out] is_valid_ret a pointer to a variable that will hold the result
 *                          of the check. Result will be CCS_TRUE if the
 *                          features is valid. Result will be CCS_FALSE if
 *                          an hyperparameter value is not a valid value
 *                          for this hyperparameter;
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features_space is not a valid CCS features
 *                              space; or if \p features is not a valid CCS
 *                              features
 * @return #CCS_INVALID_FEATURES if \p features is not associated to the
 *                                features space; or if the number of values
 *                                contained in \p features is not equal to the
 *                                number of hyperparameters in the features
 *                                space
 */
extern ccs_error_t
ccs_features_space_check_features(ccs_features_space_t  features_space,
                                  ccs_features_t        features,
                                  ccs_bool_t           *is_valid_ret);

/**
 * Check that a set of values would create a valid features for a
 * features space.
 * @param[in] features_space
 * @param[in] num_values the number of provided values
 * @param[in] values an array of \p num_values values that would become a
 *                   features
 * @param[out] is_valid_ret a pointer to a variable that will hold the result
 *                          of the check. Result will be CCS_TRUE if the
 *                          features is valid. Result will be CCS_FALSE if
 *                          an hyperparameter value is not a valid value
 *                          for this hyperparameter;
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_OBJECT if \p features_space is not a valid CCS features
 *                              space
 * @return #CCS_INVALID_VALUE if \p values is NULL and num_values is greater
 *                             than 0
 * @return #CCS_INVALID_FEATURES if \p num_values is not equal to the number of
 *                                hyperparameters in the features space
 */
extern ccs_error_t
ccs_features_space_check_features_values(ccs_features_space_t  features_space,
                                         size_t                num_values,
                                         ccs_datum_t          *values,
                                         ccs_bool_t           *is_valid_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_FEATURES_SPACE
