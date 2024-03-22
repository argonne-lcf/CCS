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
 * Types of a CCS objective.
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
 * Create a new objective space.
 * @param[in] name pointer to a string that will be copied internally
 * @param[in] num_parameters the number of provided parameters
 * @param[in] parameters an array of \p num_parameters parameters
 *                       to add to the objective space
 * @param[in] num_objectives the number of provided expressions
 * @param[in] objectives an array o \p num_objectives expressions to add as
 *                       objectives to the objective space
 * @param[in] types an array o \p num_objectives types of objectives
 * @param[out] objective_space_ret a pointer to the variable that will hold
 *                                 the newly created objective space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name is NULL; or if \p
 * objective_space_ret is NULL; or if \p parameters is NULL; or if \p
 * num_parameters is NULL; or if \p objectives is NULL and \p
 * num_objectives is greater than 0; of if \p types is NULL and \p
 * num_objectives is greater than 0
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if a parameter is not a valid CCS
 * parameter; or if an expressions is not a valid CCS expression
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if a parameter appears more than
 * once in \p parameters; or if two or more parameters share the same name; or
 * if an expression references a parameter that is not in \p parameters
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new objective space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_objective_space(
	const char            *name,
	size_t                 num_parameters,
	ccs_parameter_t       *parameters,
	size_t                 num_objectives,
	ccs_expression_t      *objectives,
	ccs_objective_type_t  *types,
	ccs_objective_space_t *objective_space_ret);

/**
 * Check that a evaluation is a valid in a objective space.
 * @param[in] objective_space
 * @param[in] evaluation
 * @param[out] is_valid_ret a pointer to a variable that will hold the result
 *                          of the check. Result will be #CCS_TRUE if the
 *                          evaluation is valid. Result will be #CCS_FALSE
 *                          if an active parameter value is not a valid value
 *                          for this parameter; or if an inactive parameter
 *                          value is not inactive; or if a forbidden clause
 *                          would be evaluating to #ccs_true
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a
 * valid CCS objective space; or if \p evaluation is not a valid CCS
 * evaluation
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p is_valid_ret is NULL
 * @return #CCS_RESULT_ERROR_INVALID_EVALUATION if \p evaluation is not
 * associated to the objective space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_objective_space_check_evaluation(
	ccs_objective_space_t objective_space,
	ccs_evaluation_t      evaluation,
	ccs_bool_t           *is_valid_ret);

/**
 * Get the objective of rank index in a objective space.
 * @param[in] objective_space
 * @param[in] index the index of the objective to retrieve
 * @param[out] expression_ret a pointer to the variable that will contain the
 *                            returned expression
 * @param[out] type_ret a pointer to the variable that will contain the returned
 *                      objective type
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid
 * CCS objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p expression_ret or \p type_ret
 * are NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if \p index is greater than the
 * number of objectives in the objective space
 * @remarks
 *   This function is thread-safe
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
 *                           expressions. If \p expressions is not NULL, \p
 *                           num_objectives must be greater than 0
 * @param[out] expressions an array of \p num_objectives that will contain the
 *                         returned expressions, or NULL. If the array is too
 *                         big, extra values are set to NULL
 * @param[out] types an array of \p num_objectives types that will contain the
 *                   objective types
 * @param[out] num_objectives_ret a pointer to a variable that will contain the
 *                                 number of expressions that are or would be
 *                                 returned. Can be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid
 * CCS objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p expressions is NULL and \p
 * num_objectives is greater than 0; if \p types is NULL and \p num_objectives
 * is greater than 0; or if or if \p expressions is NULL and \p
 * num_objectives_ret is NULL; or if \p num_objectives is less than then number
 * of expressions that would be returned
 * @remarks
 *   This function is thread-safe
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
