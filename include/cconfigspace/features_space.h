#ifndef _CCS_FEATURES_SPACE
#define _CCS_FEATURES_SPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file features_space.h
 * A features space is a context (see context.h) defining a set of
 * parameters.
 */

/**
 * Create a new features space.
 * @param[in] name pointer to a string that will be copied internally
 * @param[out] features_space_ret a pointer to the variable that will hold
 *                                     the newly created features space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name is NULL; or if \p
 * features_space_ret is NULL; or if \p parameters is NULL; or if \p
 * num_parameters is NULL
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if a parameter is not a valid CCS
 * parameter
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if a parameter appears more than
 * once in \p parameters; or if two or more parameters share the same name
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new features space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_features_space(
	const char           *name,
	size_t                num_parameters,
	ccs_parameter_t      *parameters,
	ccs_features_space_t *features_space_ret);

/**
 * Check that a features is a valid in a features space.
 * @param[in] features_space
 * @param[in] features
 * @param[out] is_valid_ret a pointer to a variable that will hold the result
 *                          of the check. Result will be #CCS_TRUE if the
 *                          features is valid. Result will be #CCS_FALSE if
 *                          an parameter value is not a valid value
 *                          for this parameter;
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_space is not a valid
 * CCS features space; or if \p features is not a valid CCS features
 * @return #CCS_RESULT_ERROR_INVALID_FEATURES if \p features is not associated
 * to the features space; or if the number of values contained in \p features is
 * not equal to the number of parameters in the features space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_space_check_features(
	ccs_features_space_t features_space,
	ccs_features_t       features,
	ccs_bool_t          *is_valid_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_FEATURES_SPACE
