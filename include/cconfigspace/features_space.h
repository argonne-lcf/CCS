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
 * Get the name of a features space.
 * @param[in] features_space
 * @param[out] name_ret a pointer to a `char *` variable which will contain a
 *                      pointer to the features space name.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features space is not a valid
 * CCS features space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_space_get_name(
	ccs_features_space_t features_space,
	const char         **name_ret);

/**
 * Get the number of parameters in a features space.
 * @param[in] features_space
 * @param[out] num_parameters_ret a pointer to the variable that will contain
 *                                the number of parameters in the features
 *                                space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_space is not a valid
 * CCS features space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p num_parameters_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_space_get_num_parameters(
	ccs_features_space_t features_space,
	size_t              *num_parameters_ret);

/**
 * Get an parameter in a features space given its index.
 * @param[in] features_space
 * @param[in] index the index of the parameter to retrieve
 * @param[out] parameter_ret a pointer to the variable that will contain the
 *                             parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_space is not a valid
 * CCS features space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p parameter_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if \p index is greater than the count
 * of parameters in the features space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_space_get_parameter(
	ccs_features_space_t features_space,
	size_t               index,
	ccs_parameter_t     *parameter_ret);

/**
 * Get an parameter in a features space given its name.
 * @param[in] features_space
 * @param[in] name the name of the parameter to retrieve
 * @param[out] parameter_ret a pointer to the variable that will contain the
 *                           parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_space is not a valid
 * CCS features space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name or \p parameter_ret are
 * NULL
 * @return #CCS_RESULT_ERROR_INVALID_NAME if no parameter with such \p name
 * exist in the \p features space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_space_get_parameter_by_name(
	ccs_features_space_t features_space,
	const char          *name,
	ccs_parameter_t     *parameter_ret);

/**
 * Get the index of an parameter in the features space given its name.
 * @param[in] features_space
 * @param[in] name the name of the parameter to retrieve the index of
 * @param[out] index_ret a pointer to the variable that will contain the index
 *                       of parameter in the \p features_space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_space is not a valid
 * CCS features space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name or \p index_ret are NULL
 * @return #CCS_RESULT_ERROR_INVALID_NAME if no parameter with such \p name
 * exist in the features space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_space_get_parameter_index_by_name(
	ccs_features_space_t features_space,
	const char          *name,
	size_t              *index_ret);

/**
 * Get the index of an parameter in the features space.
 * @param[in] features_space
 * @param[in] parameter
 * @param[out] index_ret a pointer to the variable which will contain the index
 *                       of the parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_space is not a valid
 * CCS features space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p index_ret is NULL
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if \p features_space does not
 * contain \p parameter
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_space_get_parameter_index(
	ccs_features_space_t features_space,
	ccs_parameter_t      parameter,
	size_t              *index_ret);

/**
 * Get the indices of a set of parameters in a features space.
 * @param[in] features_space
 * @param[in] num_parameters the number of parameters to query the index for
 * @param[in] parameters an array of \p num_parameters parameters to query the
 *                       index for
 * @param[out] indexes an array of \p num_parameters indices that will
 *                     contain the values of the parameter indices
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_space is not a valid
 * CCS features space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p parameters is NULL and \p
 * num_parameters is greater than 0; or if \p indexes is NULL and \p
 * num_parameters is greater than 0
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if at least one of the
 * parameters is not contained in \p features_space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_space_get_parameter_indexes(
	ccs_features_space_t features_space,
	size_t               num_parameters,
	ccs_parameter_t     *parameters,
	size_t              *indexes);

/**
 * Get the parameters in the given features space.
 * @param[in] features_space
 * @param[in] num_parameters is the number of parameters that can be added to
 *                           \p parameters. If \p parameters is not NULL \p
 *                           num_parameters must be greater than 0
 * @param[in] parameters an array of \p num_parameters that will contain the
 *                       returned parameters or NULL. If the array is too big,
 *                       extra values are set to NULL
 * @param[out] num_parameters_ret a pointer to a variable that will contain the
 *                                number of parameters that are or would be
 *                                returned. Can be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_space is not a valid
 * CCS features space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p parameters is NULL and \p
 * num_parameters is greater than 0; or if \p parameters is NULL and
 * num_parameters_ret is NULL; or if \p num_parameters is less than the number
 * of parameters that would be returned
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_space_get_parameters(
	ccs_features_space_t features_space,
	size_t               num_parameters,
	ccs_parameter_t     *parameters,
	size_t              *num_parameters_ret);

/**
 * Validate that a given value at the given index is valid in a features
 * space, and return a sanitized value.
 * @param[in] features_space
 * @param[in] index the index of the value in the features_space
 * @param[in] value the datum to validate
 * @param[out] value_ret a pointer that will contain the validated value. If \p
 *                       value is a string \p value_ret will contain a non
 *                       transient string.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_space is not a valid
 * CCS features space
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if index is greater than the number
 * of parameters in \p features_space
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory while
 * memoizing a string
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if the value did not validate or if
 * value_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_space_validate_value(
	ccs_features_space_t features_space,
	size_t               index,
	ccs_datum_t          value,
	ccs_datum_t         *value_ret);

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

/**
 * Check that a set of values would create a valid features for a
 * features space.
 * @param[in] features_space
 * @param[in] num_values the number of provided values
 * @param[in] values an array of \p num_values values that would become a
 *                   features
 * @param[out] is_valid_ret a pointer to a variable that will hold the result
 *                          of the check. Result will be #CCS_TRUE if the
 *                          features is valid. Result will be #CCS_FALSE if
 *                          an parameter value is not a valid value
 *                          for this parameter;
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_space is not a valid
 * CCS features space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p values is NULL and num_values
 * is greater than 0
 * @return #CCS_RESULT_ERROR_INVALID_FEATURES if \p num_values is not equal to
 * the number of parameters in the features space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_space_check_features_values(
	ccs_features_space_t features_space,
	size_t               num_values,
	ccs_datum_t         *values,
	ccs_bool_t          *is_valid_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_FEATURES_SPACE
