#ifndef _CCS_BINDING_H
#define _CCS_BINDING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file binding.h
 * A Binding is set of value in a Context see context.h. Those values can be
 * accessed by using the parameter index in the context. Bindings are immutable
 * except from a reference counting and callback management point of view.
 */

/**
 * Get the context of a binding.
 * @param[in] binding
 * @param[out] context_ret a pointer to the variable which will contain the
 *                         context
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p binding is not a valid CCS
 * object
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p context_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_binding_get_context(ccs_binding_t binding, ccs_context_t *context_ret);

/**
 * Get the value of the parameter at the given index.
 * @param[in] binding
 * @param[in] index index of the parameter in the associated context
 * @param[out] value_ret a pointer to the variable that will hold the value
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p binding is not a valid CCS
 * object
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p value_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if \p index is greater than the count
 * of parameters in the context
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_binding_get_value(
	ccs_binding_t binding,
	size_t        index,
	ccs_datum_t  *value_ret);

/**
 * Get all the values in the binding.
 * @param[in] binding
 * @param[in] num_values the size of the \p values array
 * @param[out] values an array of size \p num_values to hold the returned values
 *                    or NULL. If the array is too big, extra values are set to
 *                    #CCS_DATA_TYPE_NONE
 * @param[out] num_values_ret a pointer to a variable that will contain the
 *                            number of values that are or would be returned.
 *                            Can be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p binding is not a valid CCS
 * object
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p values is NULL and \p
 * num_values is greater than 0; or if \p values is NULL and \p num_values_ret
 * is NULL; or if \p num_values is less than the number of values that would be
 * returned
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_binding_get_values(
	ccs_binding_t binding,
	size_t        num_values,
	ccs_datum_t  *values,
	size_t       *num_values_ret);

/**
 * Get the value of the parameter with the given name.
 * @param[in] binding
 * @param[in] name the name of the parameter whose value to retrieve
 * @param[out] found_ret a pointer to the an optional variable that will
 *                       hold wether a parameter named \p name was found in
 *                       \p binding
 * @param[out] value_ret a pointer to the variable that will hold the value
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p binding is not a valid CCS
 * object
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p value_ret is NULL
 * @return #CCS_RESULT_ERROR_INVALID_NAME if \p found_ret is NULL  no parameter
 * with such \p name exist in the \p binding context
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_binding_get_value_by_name(
	ccs_binding_t binding,
	const char   *name,
	ccs_bool_t   *found_ret,
	ccs_datum_t  *value_ret);

/**
 * Get the value of the parameter with the given handle.
 * @param[in] binding
 * @param[in] parameter parameter whose value to retrieve
 * @param[out] found_ret a pointer to the an optional variable that will
 *                       hold wether the parameter was found in the \p
 *                       binding context
 * @param[out] value_ret a pointer to the variable that will hold the value
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p binding is not a valid CCS
 * object
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if \p found_ret is NULL
 * and \p parameter does not exist in \p binding context
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_binding_get_value_by_parameter(
	ccs_binding_t   binding,
	ccs_parameter_t parameter,
	ccs_bool_t     *found_ret,
	ccs_datum_t    *value_ret);

/**
 * Compute a hash value for the binding by hashing together the context
 * reference, the number of values, and the values themselves.
 * @param[in] binding
 * @param[out] hash_ret the address of the variable that will contain the hash
 *                      value.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p binding is not a valid CCS
 * binding
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p hash_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_binding_hash(ccs_binding_t binding, ccs_hash_t *hash_ret);

/**
 * Define a strict ordering of binding instances. Context, number of values and
 * values are compared.
 * @param[in] binding the first binding
 * @param[in] other_binding the second binding
 * @param[out] cmp_ret the pointer to the variable that will contain the result
 *                     of the comparison. Will contain -1, 0, or 1 depending on
 *                     if the first binding is found to be respectively lesser
 *                     than, equal, or greater then the second binding
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p binding or \p other_binding
 * are not valid CCS bindings; or if \p binding and \p other_binding are
 * bindings of different types
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p cmp_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_binding_cmp(
	ccs_binding_t binding,
	ccs_binding_t other_binding,
	int          *cmp_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_BINDING_H
