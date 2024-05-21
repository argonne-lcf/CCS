#ifndef _CCS_FEATURE_SPACE
#define _CCS_FEATURE_SPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file feature_space.h
 * A feature space is a context (see context.h) defining a set of
 * features represented using CCS parameters (see parameter.h).
 */

/**
 * Create a new feature space.
 * @param[in] name pointer to a string that will be copied internally
 * @param[in] num_parameters the number of provided parameters
 * @param[in] parameters an array of \p num_parameters parameters
 *                       to add to the feature space
 * @param[out] feature_space_ret a pointer to the variable that will hold
 *                                     the newly created feature space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name is NULL; or if \p
 * feature_space_ret is NULL; or if \p parameters is NULL; or if \p
 * num_parameters is NULL
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if a parameter is not a valid CCS
 * parameter
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if a parameter appears more than
 * once in \p parameters; or if two or more parameters share the same name; or
 * if a paramater is already part of another context
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new feature space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_feature_space(
	const char          *name,
	size_t               num_parameters,
	ccs_parameter_t     *parameters,
	ccs_feature_space_t *feature_space_ret);

/**
 * Get the default features of a feature space.
 * @param[in] feature_space
 * @param[in] features_ret a pointer to a variable that will contain the
 *                         returned features.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p feature_space is not a valid
 * CCS feature space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p feature_space_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_feature_space_get_default_features(
	ccs_feature_space_t feature_space,
	ccs_features_t     *features_ret);

/**
 * Check that a features is a valid in a feature space.
 * @param[in] feature_space
 * @param[in] features
 * @param[out] is_valid_ret a pointer to a variable that will hold the result
 *                          of the check. Result will be #CCS_TRUE if the
 *                          features is valid. Result will be #CCS_FALSE if
 *                          an parameter value is not a valid value
 *                          for this parameter;
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p feature_space is not a valid
 * CCS feature space; or if \p features is not a valid CCS features
 * @return #CCS_RESULT_ERROR_INVALID_FEATURES if \p features is not associated
 * to the feature space; or if the number of values contained in \p features is
 * not equal to the number of parameters in the feature space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_feature_space_check_features(
	ccs_feature_space_t feature_space,
	ccs_features_t      features,
	ccs_bool_t         *is_valid_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_FEATURE_SPACE
