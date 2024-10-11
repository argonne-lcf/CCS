#ifndef _CCS_CONTEXT_H
#define _CCS_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file context.h
 * A Context is a collection of parameters defining a value space. Each
 * parameter has a specific index that can be used to reference it.
 * The methods defined in this file can be used on objects who are contexts.
 * In practice those are useful for binding to avoid binding the children
 * methods, whereas a C application would rather use the object class specific
 * versions in order to benefit from the added type safety.
 */

/**
 * Get the name of a context.
 * @param[in] context
 * @param[out] name_ret a pointer to a `char *` variable which will contain a
 *                      pointer to the context name.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p context is not a valid CCS
 * object
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_context_get_name(ccs_context_t context, const char **name_ret);

/**
 * Get an parameter in a context given its index.
 * @param[in] context
 * @param[in] index the index of the parameter to retrieve
 * @param[out] parameter_ret a pointer to the variable that will contain the
 *                           parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p context is not a valid CCS
 * object
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p parameter_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if \p index is greater than the count
 * of parameters in the context
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_context_get_parameter(
	ccs_context_t    context,
	size_t           index,
	ccs_parameter_t *parameter_ret);

/**
 * Get the parameters in the given context.
 * @param[in] context
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
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p context is not a valid CCS
 * object
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p parameters is NULL and \p
 * num_parameters is greater than 0; or if \p parameters is NULL and \p
 * num_parameters_ret is NULL; or if \p num_parameters is less than the number
 * of parameters that would be returned
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_context_get_parameters(
	ccs_context_t    context,
	size_t           num_parameters,
	ccs_parameter_t *parameters,
	size_t          *num_parameters_ret);

/**
 * Get an parameter in a context given its name.
 * @param[in] context
 * @param[in] name the name of the parameter to retrieve
 * @param[out] parameter_ret a pointer to the variable that will contain the
 *                           parameter, or NULL if the parameter is not found
 *                           in the context
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p context is not a valid CCS
 * object
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name or \p parameter_ret are
 * NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_context_get_parameter_by_name(
	ccs_context_t    context,
	const char      *name,
	ccs_parameter_t *parameter_ret);

/**
 * Get the index of an parameter in the context given its name.
 * @param[in] context
 * @param[in] name the name of the parameter to retrieve the index of
 * @param[out] found_ret a pointer to the an optional variable that will
 *                       hold wether a parameter named \p name was found in
 *                       \p context
 * @param[out] index_ret a pointer to the variable that will contain the index
 *                       of parameter in the \p context
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p context is not a valid CCS
 * object
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name or \p index_ret are NULL
 * @return #CCS_RESULT_ERROR_INVALID_NAME if \p found_ret is NULL and no
 * parameter with such \p name exist in the context
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_context_get_parameter_index_by_name(
	ccs_context_t context,
	const char   *name,
	ccs_bool_t   *found_ret,
	size_t       *index_ret);

/**
 * Get the index of an parameter in the context.
 * @param[in] context
 * @param[in] parameter
 * @param[out] found_ret a pointer to the an optional variable that will
 *                       hold wether the parameter was found in the \p
 *                       context
 * @param[out] index_ret a pointer to the variable which will contain the index
 *                       of the parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p context is not a valid CCS
 * object;
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p index_ret is NULL
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if \p found_ret is NULL and
 * \p context does not contain \p parameter
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_context_get_parameter_index(
	ccs_context_t   context,
	ccs_parameter_t parameter,
	ccs_bool_t     *found_ret,
	size_t         *index_ret);

/**
 * Get the indices of a set of parameters in a context.
 * @param[in] context
 * @param[in] num_parameters the number of parameters to query the index for
 * @param[in] parameters an array of \p num_parameters parameters to query the
 *                       index for
 * @param[out] found an optional array of \p num_parameters variables that
		     will hold wether the parameter was found in \p context
 * @param[out] indexes an array of \p num_parameters indices that will
 *                     contain the values of the parameter indices
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p context is not a valid CCS
 * object
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p parameters is NULL and \p
 * num_parameters is greater than 0; or if \p indexes is NULL and \p
 * num_parameters is greater than 0
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if \p found_ret is NULL and
 * at least one of the parameters is not contained in \p context
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_context_get_parameter_indexes(
	ccs_context_t    context,
	size_t           num_parameters,
	ccs_parameter_t *parameters,
	ccs_bool_t      *found,
	size_t          *indexes);

/**
 * Validate that a given value at the given index is valid in the context, and
 * return a sanitized value.
 * @param[in] context
 * @param[in] index the index of the value in the context
 * @param[in] value the datum to validate
 * @param[out] value_ret a pointer that will contain the validated value. If \p
 *                       value is a string \p value_ret will contain a non
 *                       transient string.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p context is not a valid CCS
 * object
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if index is greater than the number
 * of parameters in \p context
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory while
 * memoizing a string
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if the value did not validate or if
 * value_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_context_validate_value(
	ccs_context_t context,
	size_t        index,
	ccs_datum_t   value,
	ccs_datum_t  *value_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_CONTEXT_H
