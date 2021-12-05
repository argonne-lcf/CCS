#ifndef _CCS_FEATURES
#define _CCS_FEATURES

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file features.h
 * A features is a binding (see binding.h) over a features space (see
 * features_space.h).
 */

/**
 * Create a new instance of a features on a given features space. If no values
 * are provided the values will be initialized to #CCS_NONE.
 * @param[in] features_space
 * @param[in] num_values the number of provided values to initialize the
 *                       features instance
 * @param[in] values an optional array of values to initialize the features
 *                   user_data a pointer to the user data to attach to this
 *                   features instance
 * @param[in] user_data a pointer to the user data to attach to this features
 *                      instance
 * @param[out] features_ret a pointer to the variable that will hold the newly
 *                          created features
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p features is not a valid CCS features space
 * @return -#CCS_INVALID_VALUE if \p features_ret is NULL; or if \p values is
 *                             NULL and \p num_values is greater than 0; or if
 *                             the number of values provided is not to the
 *                             number of hyperparameters in the features space
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             features
 */
extern ccs_result_t
ccs_create_features(ccs_features_space_t features_space,
                    size_t               num_values,
                    ccs_datum_t         *values,
                    void                *user_data,
                    ccs_features_t      *features_ret);

/**
 * Get the associated features space.
 * @param[in] features
 * @param[out] features_space_ret a pointer to the variable that will
 *                                contain the features space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p features is not a valid CCS features
 * @return -#CCS_INVALID_VALUE if \p features_space_ret is NULL
 */
extern ccs_result_t
ccs_features_get_features_space(ccs_features_t        features,
                                ccs_features_space_t *features_space_ret);

/**
 * Get the associated `user_data` pointer.
 * @param[in] features
 * @param[out] user_data_ret a pointer to `void *` variable that will contain
 *                           the value of the `user_data`
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p features is not a valid CCS features
 * @return -#CCS_INVALID_VALUE if \p user_data_ret is NULL
 */
extern ccs_result_t
ccs_features_get_user_data(ccs_features_t   features,
                           void           **user_data_ret);

/**
 * Get the value of the hyperparameter at the given index.
 * @param[in] features
 * @param[in] index index of the hyperparameter in the associated features space
 * @param[out] value_ret a pointer to the variable that will hold the value
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p features is not a valid CCS features
 * @return -#CCS_INVALID_VALUE if \p value_ret is NULL
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the count of
 *                             hyperparameters in the features space
 */
extern ccs_result_t
ccs_features_get_value(ccs_features_t  features,
                       size_t          index,
                       ccs_datum_t    *value_ret);

/**
 * Set the value of the hyperparameter at the given index. Transient values will
 * be validated and memoized if needed.
 * @param[in,out] features
 * @param[in] index index of the hyperparameter in the associated features space
 * @param[in] value the value
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p features is not a valid CCS features
 * @return -#CCS_INVALID_VALUE if \p value_ret is NULL
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the count of
 *                             hyperparameters in the features space
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory while memoizing a
 *                             string
 */
extern ccs_result_t
ccs_features_set_value(ccs_features_t features,
                       size_t         index,
                       ccs_datum_t    value);

/**
 * Get all the values in the features.
 * @param[in] features
 * @param[in] num_values the size of the \p values array
 * @param[out] values an array of size \p num_values to hold the returned
 *                    values, or NULL. If the array is too big, extra values
 *                    are set to #CCS_NONE
 * @param[out] num_values_ret a pointer to a variable that will contain the
 *                            number of values that are or would be returned.
 *                            Can be NULL
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p features is not a valid CCS features
 * @return -#CCS_INVALID_VALUE if \p values is NULL and \p num_values is greater
 *                             than 0; or if \p values is NULL and
 *                             num_values_ret is NULL; or if \p num_values is
 *                             less than the number of values that would be
 *                             returned
 */
extern ccs_result_t
ccs_features_get_values(ccs_features_t  features,
                        size_t          num_values,
                        ccs_datum_t    *values,
                        size_t         *num_values_ret);

/**
 * Get the value of the hyperparameter with the given name.
 * @param[in] features
 * @param[in] name the name of the hyperparameter whose value to retrieve
 * @param[out] value_ret a pointer to the variable that will hold the value
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p features is not a valid CCS features
 * @return -#CCS_INVALID_VALUE if \p value_ret is NULL
 * @return -#CCS_INVALID_NAME if no hyperparameter with such \p name exist in
 *                            the \p features space
 */
extern ccs_result_t
ccs_features_get_value_by_name(ccs_features_t  features,
                               const char     *name,
                               ccs_datum_t    *value_ret);

/**
 * Check that the features is a valid features for the features space.
 * @param[in] features
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p features is not a valid CCS features
 * @return -#CCS_INVALID_CONFIGURATION if \p features is found to be invalid
 */
extern ccs_result_t
ccs_features_check(ccs_features_t features);

/**
 * Compute a hash value for the features by hashing together the features space
 * reference, the number of values, and the values themselves.
 * @param[in] features
 * @param[out] hash_ret the address of the variable that will contain the hash
 *                      value
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p features is not a valid CCS features
 * @return -#CCS_INVALID_VALUE if \p hash_ret is NULL
 */
extern ccs_result_t
ccs_features_hash(ccs_features_t  features,
                  ccs_hash_t     *hash_ret);

/**
 * Define a strict ordering of features instances. Configuration space, number
 * of values and values are compared.
 * @param[in] features the first features
 * @param[in] other_features the second features
 * @param[out] cmp_ret the pointer to the variable that will contain the result
 *                     of the comparison. Will contain -1, 0, or 1 depending on
 *                     if the first features is found to be respectively lesser
 *                     than, equal, or greater then the second features
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p features or \p other_features are not a
 *                              valid CCS object
 */
extern ccs_result_t
ccs_features_cmp(ccs_features_t  features,
                 ccs_features_t  other_features,
                 int            *cmp_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_FEATURES
