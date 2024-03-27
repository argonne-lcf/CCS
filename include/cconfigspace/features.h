#ifndef _CCS_FEATURES
#define _CCS_FEATURES

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file features.h
 * A features is a binding (see binding.h) over a feature space (see
 * feature_space.h).
 */

/**
 * Create a new instance of a features on a given feature space. If no values
 * are provided the values will be initialized to #CCS_DATA_TYPE_NONE.
 * @param[in] feature_space
 * @param[in] num_values the number of provided values to initialize the
 *                       features instance
 * @param[in] values an optional array of values to initialize the features
 * @param[out] features_ret a pointer to the variable that will hold the newly
 *                          created features
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features is not a valid CCS
 * feature space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p features_ret is NULL; or if \p
 * values is NULL and \p num_values is greater than 0; or if the number of
 * values provided is not to the number of parameters in the feature space
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new features
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_features(
	ccs_feature_space_t feature_space,
	size_t              num_values,
	ccs_datum_t        *values,
	ccs_features_t     *features_ret);

/**
 * Get the associated feature space.
 * @param[in] features
 * @param[out] feature_space_ret a pointer to the variable that will
 *                                contain the feature space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features is not a valid CCS
 * features
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p feature_space_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_get_feature_space(
	ccs_features_t       features,
	ccs_feature_space_t *feature_space_ret);

/**
 * Check that the features is a valid features for the feature space.
 * @param[in] features
 * @param[out] is_valid_ret a pointer to a variable that will hold the result
 *                          of the check. Result will be #CCS_TRUE if the
 *                          features is valid. Result will be #CCS_FALSE if
 *                          an parameter value is not a valid value
 *                          for this parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features is not a valid CCS
 * features
 * @return #CCS_RESULT_ERROR_INVALID_CONFIGURATION if \p features is found to be
 * invalid
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_check(ccs_features_t features, ccs_bool_t *is_valid_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_FEATURES
