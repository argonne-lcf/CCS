#ifndef _CCS_OBJECTIVE_SPACE
#define _CCS_OBJECTIVE_SPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file objective_space.h
 * An objective space is a context (see context.h) defining a set of
 * parameters. Objective space also define a list of expressions (see
 * expression.h) over those parameters called objectives.
 */

/**
 * Types of CCS objetives.
 */
enum ccs_objective_type_e {
	/** Objective should be minimized */
	CCS_OBJECTIVE_TYPE_MINIMIZE,
	/** Objective should be maximized */
	CCS_OBJECTIVE_TYPE_MAXIMIZE,
	/** Guard */
	CCS_OBJECTIVE_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_OBJECTIVE_TYPE_FORCE_32BIT = INT_MAX
};

/**
 * A commodity type to represent objective types.
 */
typedef enum ccs_objective_type_e ccs_objective_type_t;

/**
 * Create a new empty objective space.
 * @param[in] name pointer to a string that will be copied internally
 * @param[out] objective_space_ret a pointer to the variable that will hold
 *                                     the newly created objective space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name is NULL; or if \p
 *                             objective_space_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             objective space
 */
extern ccs_result_t
ccs_create_objective_space(
	const char            *name,
	ccs_objective_space_t *objective_space_ret);

/**
 * Get the name of a objective space.
 * @param[in] objective_space
 * @param[out] name_ret a pointer to a `char *` variable which will contain a
 *                      pointer to the objective space name.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective space is not a valid CCS
 *                              objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name_ret is NULL
 */
extern ccs_result_t
ccs_objective_space_get_name(
	ccs_objective_space_t objective_space,
	const char          **name_ret);

/**
 * Add a parameter to the objective space.
 * @param[in,out] objective_space
 * @param[in] parameter the parameter to add to the objective
 *                           space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space; or \p parameter is not a
 *                              valid CCS parameter
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if \p parameter is already in the
 *                                      objective space; or if a parameter
 *                                      with the same name already exists in the
 *                                      objective space
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if a memory could not be allocated to store
 *                             the additional parameter and associated data
 *                             structures
 */
extern ccs_result_t
ccs_objective_space_add_parameter(
	ccs_objective_space_t objective_space,
	ccs_parameter_t       parameter);

/**
 * Add parameters to the objective space.
 * @param[in,out] objective_space
 * @param[in] num_parameters the number of provided parameters
 * @param[in] parameters an array of \p num_parameters parameters
 *                            to add to the objective space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space; or a parameter is not a
 *                              valid CCS parameter
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p parameters is NULL and \p
 *                             num_parameters is greater than 0
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if a parameter is already in the
 *                                      objective space; or if a parameter
 *                                      with the same name already exists in the
 *                                      objective space
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if memory could not be allocated to store
 *                             additional parameters and associated data
 *                             structures
 */
extern ccs_result_t
ccs_objective_space_add_parameters(
	ccs_objective_space_t objective_space,
	size_t                num_parameters,
	ccs_parameter_t      *parameters);

/**
 * Get the number of parameters in a objective space.
 * @param[in] objective_space
 * @param[out] num_parameters_ret a pointer to the variable that will
 *                                     contain the number of parameters in
 *                                     the objective space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p num_parameters_ret is NULL
 */
extern ccs_result_t
ccs_objective_space_get_num_parameters(
	ccs_objective_space_t objective_space,
	size_t               *num_parameters_ret);

/**
 * Get an parameter in a objective space given its index.
 * @param[in] objective_space
 * @param[in] index the index of the parameter to retrieve
 * @param[out] parameter_ret a pointer to the variable that will contain
 *                                the parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p parameter_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if \p index is greater than the count of
 *                             parameters in the objective space
 */
extern ccs_result_t
ccs_objective_space_get_parameter(
	ccs_objective_space_t objective_space,
	size_t                index,
	ccs_parameter_t      *parameter_ret);

/**
 * Get an parameter in a objective space given its name.
 * @param[in] objective_space
 * @param[in] name the name of the parameter to retrieve
 * @param[out] parameter_ret a pointer to the variable that will contain
 *                                the parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name or \p parameter_ret are NULL
 * @return #CCS_RESULT_ERROR_INVALID_NAME if no parameter with such \p name exist in
 *                            the \p objective space
 */
extern ccs_result_t
ccs_objective_space_get_parameter_by_name(
	ccs_objective_space_t objective_space,
	const char           *name,
	ccs_parameter_t      *parameter_ret);

/**
 * Get the index of an parameter in the objective space given its name.
 * @param[in] objective_space
 * @param[in] name the name of the parameter to retrieve the index of
 * @param[out] index_ret a pointer to the variable that will contain the index
 *                       of parameter in the \p objective_space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name or \p index_ret are NULL
 * @return #CCS_RESULT_ERROR_INVALID_NAME if no parameter with such \p name exist in
 *                            the objective space
 */
extern ccs_result_t
ccs_objective_space_get_parameter_index_by_name(
	ccs_objective_space_t objective_space,
	const char           *name,
	size_t               *index_ret);

/**
 * Get the index of an parameter in the objective space.
 * @param[in] objective_space
 * @param[in] parameter
 * @param[out] index_ret a pointer to the variable which will contain the index
 *                       of the parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p index_ret is NULL
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if \p objective_space does not
 *                                      contain \p parameter
 */
extern ccs_result_t
ccs_objective_space_get_parameter_index(
	ccs_objective_space_t objective_space,
	ccs_parameter_t       parameter,
	size_t               *index_ret);

/**
 * Get the indices of a set of parameters in a objective space.
 * @param[in] objective_space
 * @param[in] num_parameters the number of parameters to query the
 *                                index for
 * @param[in] parameters an array of \p num_parameters parameters
 *                            to query the index for
 * @param[out] indexes an array of \p num_parameters indices that will
 *                     contain the values of the parameter indices
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p parameters is NULL and \p
 *                             num_parameters is greater than 0; or if \p
 *                             indexes is NULL and \p num_parameters is
 *                             greater than 0
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if at least one of the parameters
 *                                      is not contained in \p objective_space
 */
extern ccs_result_t
ccs_objective_space_get_parameter_indexes(
	ccs_objective_space_t objective_space,
	size_t                num_parameters,
	ccs_parameter_t      *parameters,
	size_t               *indexes);

/**
 * Get the parameters in the given objective space.
 * @param[in] objective_space
 * @param[in] num_parameters is the number of parameters that can be
 *                                added to \p parameters. If \p
 *                                parameters is not NULL \p
 *                                num_parameters must be greater than 0
 * @param[in] parameters an array of \p num_parameters that will
 *                            contain the returned parameters or NULL. If
 *                            the array is too big, extra values are set to NULL
 * @param[out] num_parameters_ret a pointer to a variable that will contain
 *                                     the number of parameters that are or
 *                                     would be returned. Can be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p parameters is NULL and \p
 *                             num_parameters is greater than 0; or if \p
 *                             parameters is NULL and
 *                             num_parameters_ret is NULL; or if \p
 *                             num_parameters is less than the number of
 *                             parameters that would be returned
 */
extern ccs_result_t
ccs_objective_space_get_parameters(
	ccs_objective_space_t objective_space,
	size_t                num_parameters,
	ccs_parameter_t      *parameters,
	size_t               *num_parameters_ret);

/**
 * Check that a set of values would create a valid evaluation for an
 * objective space.
 * @param[in] objective_space
 * @param[in] num_values the number of provided values
 * @param[in] values an array of \p num_values values that would become an
 *                   evaluation
 * @param[out] is_valid_ret a pointer to a variable that will hold the result
 *                          of the check. Result will be CCS_TRUE if the
 *                          evaluation is valid. Result will be CCS_FALSE if
 *                          an parameter value is not a valid value
 *                          for this parameter;
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p values is NULL and num_values is greater
 *                             than 0
 * @return #CCS_RESULT_ERROR_INVALID_EVALUATION if \p num_values is not equal to the number
 *                                  of parameters in the objective space
 */
extern ccs_result_t
ccs_objective_space_check_evaluation_values(
	ccs_objective_space_t objective_space,
	size_t                num_values,
	ccs_datum_t          *values,
	ccs_bool_t           *is_valid_ret);

/**
 * Validate that a given value at the given index is valid in a objective
 * space, and return a sanitized value.
 * @param[in] objective_space
 * @param[in] index the index of the value in the objective_space
 * @param[in] value the datum to validate
 * @param[out] value_ret a pointer that will contain the validated value. If \p
 *                       value is a string \p value_ret will contain a non
 *                       transient string.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if index is greater than the number of
 *                             parameters in \p objective_space
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory while memoizing a
 *                             string
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if the value did not validate or if value_ret is
 *                             NULL
 */
extern ccs_result_t
ccs_objective_space_validate_value(
	ccs_objective_space_t objective_space,
	size_t                index,
	ccs_datum_t           value,
	ccs_datum_t          *value_ret);

/**
 * Add an objective to an objective space.
 * @param[in,out] objective_space
 * @param[in] expression the forbidden clause to dd to the configuration space
 * @param[in] type the type of the objective, either #CCS_OBJECTIVE_TYPE_MAXIMIZE or
 *                 #CCS_OBJECTIVE_TYPE_MINIMIZE
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space; or if \p expression is not
 *                              a valid CCS expression
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if expression references a
 *                                      parameter that is not in the
 *                                      objective space
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to allocate
 *                             internal data structures
 */
extern ccs_result_t
ccs_objective_space_add_objective(
	ccs_objective_space_t objective_space,
	ccs_expression_t      expression,
	ccs_objective_type_t  type);

/**
 * Add a list of objectives to an objective space.
 * @param[in,out] objective_space
 * @param[in] num_objectives the number of provided expressions
 * @param[in] expressions an array o \p num_objectives expressions to add as
 *                        objectives to the objective space
 * @param[in] types an array o \p num_objectives types of objectives
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space; or if at least one of the
 *                              provided expressions is not a valid CCS
 *                              expression
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p expressions is NULL and \p num_objectives
 *                             is greater than 0
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if an expression references a
 *                                      parameter that is not in the
 *                                      objective space
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to allocate
 *                             internal data structures
 */
extern ccs_result_t
ccs_objective_space_add_objectives(
	ccs_objective_space_t objective_space,
	size_t                num_objectives,
	ccs_expression_t     *expressions,
	ccs_objective_type_t *types);

/**
 * Get the objective of rank index in a objective space.
 * @param[in] objective_space
 * @param[in] index the index of the objective to retrieve
 * @param[out] expression_ret a pointer to the variable that will contain the
 *                            returned expression
 * @param[out] type_ret a pointer to the variable that will contain the returned
 *                      objective type
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p expression_ret or \p type_ret are NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if \p index is greater than the number of
 *                             objectives in the objective space
 */
extern ccs_result_t
ccs_objective_space_get_objective(
	ccs_objective_space_t objective_space,
	size_t                index,
	ccs_expression_t     *expression_ret,
	ccs_objective_type_t *type_ret);

/**
 * Get the objectives in a objective space.
 * @param[in] objective_space
 * @param[in] num_objectives the number of expressions that can be added to \p
 *                            expressions. If \p expressions is not NULL, \p
 *                            num_objectives must be greater than 0
 * @param[out] expressions an array of \p num_objectives that will contain the
 *                         returned expressions, or NULL. If the array is too
 *                         big, extra values are set to NULL
 * @param[out] types an array of \p num_objectives types that will contain the
 *                   objective types
 * @param[out] num_objectives_ret a pointer to a variable that will contain the
 *                                 number of expressions that are or would be
 *                                 returned. Can be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p expressions is NULL and \p num_objectives
 *                             is greater than 0; if \p types is NULL and \p
 *                             num_objectives is greater than 0; or if or if \p
 *                             expressions is NULL and \p num_objectives_ret is
 *                             NULL; or if \p num_objectives is less than then
 *                             number of expressions that would be returned
 */
extern ccs_result_t
ccs_objective_space_get_objectives(
	ccs_objective_space_t objective_space,
	size_t                num_objectives,
	ccs_expression_t     *expressions,
	ccs_objective_type_t *types,
	size_t               *num_objectives_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_OBJECTIVE_SPACE
