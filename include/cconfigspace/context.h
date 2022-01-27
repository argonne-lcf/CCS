#ifndef _CCS_CONTEXT_H
#define _CCS_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file context.h
 * A Context is a collection of hyperparameters defining a value space. Each
 * hyperparameter has a specific index that can be used to reference it.
 * The methods defined in this file can be used on objects who re contexts.
 * In practice those are useful for binding to avoid binding the children
 * methods, whereas a C application would rather use the object class specific
 * versions in order to benefit from the added type safety.
 */

/**
 * Get the name of a context.
 * @param[in] context
 * @param[out] name_ret a pointer to a `char *` variable which will contain a
 *                      pointer to the context name.
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p context is not a valid CCS object
 * @return -#CCS_INVALID_VALUE if \p name_ret is NULL
 */
extern ccs_result_t
ccs_context_get_name(ccs_context_t   context,
                     const char    **name_ret);

/**
 * Get the index of an hyperparameter in the context.
 * @param[in] context
 * @param[in] hyperparameter
 * @param[out] index_ret a pointer to the variable which will contain the index
 *                       of the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p context is not a valid CCS object;
 * @return -#CCS_INVALID_VALUE if \p index_ret is NULL
 * @return -#CCS_INVALID_HYPERPARAMETER if \p context does not contain \p
 *                                      hyperparameter
 */
extern ccs_result_t
ccs_context_get_hyperparameter_index(ccs_context_t         context,
                                     ccs_hyperparameter_t  hyperparameter,
                                     size_t               *index_ret);

/**
 * Get the number of hyperparameters in the given context.
 * @param[in] context
 * @param[out] num_hyperparameters_ret a pointer to the variable which will
 *                                     contain the number of hyperparameters in
 *                                     \p context
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p context is not a valid CCS object
 * @return -#CCS_INVALID_VALUE if \p num_hyperparameters_ret is NULL
 */
extern ccs_result_t
ccs_context_get_num_hyperparameters(ccs_context_t  context,
                                    size_t        *num_hyperparameters_ret);

/**
 * Get an hyperparameter in a context given its index.
 * @param[in] context
 * @param[in] index the index of the hyperparameter to retrieve
 * @param[out] hyperparameter_ret a pointer to the variable that will contain
 *                                the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p context is not a valid CCS object
 * @return -#CCS_INVALID_VALUE if \p hyperparameter_ret is NULL
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the count of
 *                             hyperparameters in the context
 */
extern ccs_result_t
ccs_context_get_hyperparameter(ccs_context_t         context,
                               size_t                index,
                               ccs_hyperparameter_t *hyperparameter_ret);

/**
 * Get an hyperparameter in a context given its name.
 * @param[in] context
 * @param[in] name the name of the hyperparameter to retrieve
 * @param[out] hyperparameter_ret a pointer to the variable that will contain
 *                                the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p context is not a valid CCS object
 * @return -#CCS_INVALID_VALUE if \p name or \p hyperparameter_ret are NULL
 * @return -#CCS_INVALID_NAME if no hyperparameter with such \p name exist in
 *                            the \p context
 */
extern ccs_result_t
ccs_context_get_hyperparameter_by_name(ccs_context_t         context,
                                       const char *          name,
                                       ccs_hyperparameter_t *hyperparameter_ret);

/**
 * Get the index of an hyperparameter in the context given its name.
 * @param[in] context
 * @param[in] name the name of the hyperparameter to retrieve the index of
 * @param[out] index_ret a pointer to the variable that will contain the index
 *                       of hyperparameter in the \p context
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p context is not a valid CCS object
 * @return -#CCS_INVALID_VALUE if \p name or \p index_ret are NULL
 * @return -#CCS_INVALID_NAME if no hyperparameter with such \p name exist in
 *                            the context
 */
extern ccs_result_t
ccs_context_get_hyperparameter_index_by_name(ccs_context_t  context,
                                             const char    *name,
                                             size_t        *index_ret);

/**
 * Get the hyperparameters in the given context.
 * @param[in] context
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
 * @return -#CCS_INVALID_OBJECT if \p context is not a valid CCS object
 * @return -#CCS_INVALID_VALUE if \p hyperparameters is NULL and \p
 *                             num_hyperparameters is greater than 0; or if \p
 *                             hyperparameters is NULL and \p
 *                             num_hyperparameters_ret is NULL; or if
 *                             \p num_hyperparameters is less than the number of
 *                             hyperparameters that would be returned
 */
extern ccs_result_t
ccs_context_get_hyperparameters(ccs_context_t          context,
                                size_t                 num_hyperparameters,
                                ccs_hyperparameter_t  *hyperparameters,
                                size_t                *num_hyperparameters_ret);

/**
 * Get the indices of a set of hyperparameters in a context.
 * @param[in] context
 * @param[in] num_hyperparameters the number of hyperparameters to query the
 *                                index for
 * @param[in] hyperparameters an array of \p num_hyperparameters hyperparameters
 *                            to query the index for
 * @param[out] indexes an array of \p num_hyperparameters indices that will
 *                     contain the values of the hyperparamters indices
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p context is not a valid CCS object
 * @return -#CCS_INVALID_VALUE if \p hyperparameters is NULL and \p
 *                             num_hyperparameters is greater than 0; or if \p
 *                             indexes is NULL and \p num_hyperparameters is
 *                             greater than 0
 * @return -#CCS_INVALID_HYPERPARAMETER if at least one of the hyperparameters
 *                                      is not contained in \p context
 */
extern ccs_result_t
ccs_context_get_hyperparameter_indexes(
		ccs_context_t          context,
		size_t                 num_hyperparameters,
		ccs_hyperparameter_t  *hyperparameters,
		size_t                *indexes);

/**
 * Validate that a given value at the given index is valid in the context, and
 * return a sanitized value.
 * @param[in] context
 * @param[in] index the index of the value in the context
 * @param[in] value the datum to validate
 * @param[out] value_ret a pointer that will contain the validated value. If \p
 *                       value is a string \p value_ret will contain a non
 *                       transient string.
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p context is not a valid CCS object
 * @return -#CCS_OUT_OF_BOUNDS if index is greater than the number of
 *                             hyperparameters in \p context
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory while memoizing a
 *                             string
 * @return -#CCS_INVALID_VALUE if the value did not validate or if value_ret is
 *                             NULL
 */
extern ccs_result_t
ccs_context_validate_value(ccs_context_t  context,
                           size_t         index,
                           ccs_datum_t    value,
                           ccs_datum_t   *value_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_CONTEXT_H
