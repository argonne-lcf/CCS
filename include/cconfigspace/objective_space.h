#ifndef _CCS_OBJECTIVE_SPACE
#define _CCS_OBJECTIVE_SPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file objective_space.h
 * An objective space is a context (see context.h) defining a set of
 * hyperparameters. Objective space also define a list of expressions (see
 * expression.h) over those hyperparameters called objectives.
 */


/**
 * Types of CCS objetives.
 */
enum ccs_objective_type_e {
	/** Objective should be minimized */
	CCS_MINIMIZE,
	/** Objective should be maximized */
	CCS_MAXIMIZE,
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
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_VALUE if \p name is NULL; or if \p
 *                             objective_space_ret is NULL
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             objective space
 */
extern ccs_result_t
ccs_create_objective_space(const char            *name,
                           ccs_objective_space_t *objective_space_ret);

/**
 * Get the name of a objective space.
 * @param[in] objective_space
 * @param[out] name_ret a pointer to a `char *` variable which will contain a
 *                      pointer to the objective space name.
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective space is not a valid CCS
 *                              objective space
 * @return -#CCS_INVALID_VALUE if \p name_ret is NULL
 */
extern ccs_result_t
ccs_objective_space_get_name(ccs_objective_space_t   objective_space,
                             const char            **name_ret);

/**
 * Add a hyperparameter to the objective space.
 * @param[in,out] objective_space
 * @param[in] hyperparameter the hyperparameter to add to the objective
 *                           space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space; or \p hyperparameter is not a
 *                              valid CCS hyperparameter
 * @return -#CCS_INVALID_HYPERPARAMETER if \p hyperparameter is already in the
 *                                      objective space; or if a hyperparameter
 *                                      with the same name already exists in the
 *                                      objective space
 * @return -#CCS_OUT_OF_MEMORY if a memory could not be allocated to store
 *                             the additional hyperparameter and associated data
 *                             structures
 */
extern ccs_result_t
ccs_objective_space_add_hyperparameter(ccs_objective_space_t objective_space,
                                       ccs_hyperparameter_t  hyperparameter);

/**
 * Add hyperparameters to the objective space.
 * @param[in,out] objective_space
 * @param[in] num_hyperparameters the number of provided hyperparameters
 * @param[in] hyperparameters an array of \p num_hyperparameters hyperparameters
 *                            to add to the objective space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space; or a hyperparameter is not a
 *                              valid CCS hyperparameter
 * @return -#CCS_INVALID_VALUE if \p hyperparameters is NULL and \p
 *                             num_hyperparameters is greater than 0
 * @return -#CCS_INVALID_HYPERPARAMETER if a hyperparameter is already in the
 *                                      objective space; or if a hyperparameter
 *                                      with the same name already exists in the
 *                                      objective space
 * @return -#CCS_OUT_OF_MEMORY if memory could not be allocated to store
 *                             additional hyperparameters and associated data
 *                             structures
 */
extern ccs_result_t
ccs_objective_space_add_hyperparameters(
	ccs_objective_space_t  objective_space,
	size_t                 num_hyperparameters,
	ccs_hyperparameter_t  *hyperparameters);

/**
 * Get the number of hyperparameters in a objective space.
 * @param[in] objective_space
 * @param[out] num_hyperparameters_ret a pointer to the variable that will
 *                                     contain the number of hyperparameters in
 *                                     the objective space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return -#CCS_INVALID_VALUE if \p num_hyperparameters_ret is NULL
 */
extern ccs_result_t
ccs_objective_space_get_num_hyperparameters(
		ccs_objective_space_t  objective_space,
		size_t                *num_hyperparameters_ret);

/**
 * Get an hyperparameter in a objective space given its index.
 * @param[in] objective_space
 * @param[in] index the index of the hyperparameter to retrieve
 * @param[out] hyperparameter_ret a pointer to the variable that will contain
 *                                the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return -#CCS_INVALID_VALUE if \p hyperparameter_ret is NULL
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the count of
 *                             hyperparameters in the objective space
 */
extern ccs_result_t
ccs_objective_space_get_hyperparameter(ccs_objective_space_t  objective_space,
                                       size_t                 index,
                                       ccs_hyperparameter_t  *hyperparameter_ret);

/**
 * Get an hyperparameter in a objective space given its name.
 * @param[in] objective_space
 * @param[in] name the name of the hyperparameter to retrieve
 * @param[out] hyperparameter_ret a pointer to the variable that will contain
 *                                the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return -#CCS_INVALID_VALUE if \p name or \p hyperparameter_ret are NULL
 * @return -#CCS_INVALID_NAME if no hyperparameter with such \p name exist in
 *                            the \p objective space
 */
extern ccs_result_t
ccs_objective_space_get_hyperparameter_by_name(
		ccs_objective_space_t  objective_space,
		const char *           name,
		ccs_hyperparameter_t  *hyperparameter_ret);

/**
 * Get the index of an hyperparameter in the objective space given its name.
 * @param[in] objective_space
 * @param[in] name the name of the hyperparameter to retrieve the index of
 * @param[out] index_ret a pointer to the variable that will contain the index
 *                       of hyperparameter in the \p objective_space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return -#CCS_INVALID_VALUE if \p name or \p index_ret are NULL
 * @return -#CCS_INVALID_NAME if no hyperparameter with such \p name exist in
 *                            the objective space
 */
extern ccs_result_t
ccs_objective_space_get_hyperparameter_index_by_name(
		ccs_objective_space_t  objective_space,
		const char            *name,
		size_t                *index_ret);

/**
 * Get the index of an hyperparameter in the objective space.
 * @param[in] objective_space
 * @param[in] hyperparameter
 * @param[out] index_ret a pointer to the variable which will contain the index
 *                       of the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return -#CCS_INVALID_VALUE if \p index_ret is NULL
 * @return -#CCS_INVALID_HYPERPARAMETER if \p objective_space does not
 *                                      contain \p hyperparameter
 */
extern ccs_result_t
ccs_objective_space_get_hyperparameter_index(
		ccs_objective_space_t  objective_space,
		ccs_hyperparameter_t   hyperparameter,
		size_t                *index_ret);

/**
 * Get the indices of a set of hyperparameters in a objective space.
 * @param[in] objective_space
 * @param[in] num_hyperparameters the number of hyperparameters to query the
 *                                index for
 * @param[in] hyperparameters an array of \p num_hyperparameters hyperparameters
 *                            to query the index for
 * @param[out] indexes an array of \p num_hyperparameters indices that will
 *                     contain the values of the hyperparamters indices
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return -#CCS_INVALID_VALUE if \p hyperparameters is NULL and \p
 *                             num_hyperparameters is greater than 0; or if \p
 *                             indexes is NULL and \p num_hyperparameters is
 *                             greater than 0
 * @return -#CCS_INVALID_HYPERPARAMETER if at least one of the hyperparameters
 *                                      is not contained in \p objective_space
 */
extern ccs_result_t
ccs_objective_space_get_hyperparameter_indexes(
		ccs_objective_space_t  objective_space,
		size_t                 num_hyperparameters,
		ccs_hyperparameter_t  *hyperparameters,
		size_t                *indexes);

/**
 * Get the hyperparameters in the given objective space.
 * @param[in] objective_space
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
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return -#CCS_INVALID_VALUE if \p hyperparameters is NULL and \p
 *                             num_hyperparameters is greater than 0; or if \p
 *                             hyperparameters is NULL and
 *                             num_hyperparameters_ret is NULL; or if \p
 *                             num_hyperparameters is less than the number of
 *                             hyperparameters that would be returned
 */
extern ccs_result_t
ccs_objective_space_get_hyperparameters(ccs_objective_space_t  objective_space,
                                        size_t                 num_hyperparameters,
                                        ccs_hyperparameter_t  *hyperparameters,
                                        size_t                *num_hyperparameters_ret);

/**
 * Check that an evaluation is a valid in an objective space.
 * @param[in] objective_space
 * @param[in] evaluation
 * @param[out] is_valid_ret a pointer to a variable that will hold the result
 *                          of the check. Result will be CCS_TRUE if the
 *                          evaluation is valid. Result will be CCS_FALSE if
 *                          an hyperparameter value is not a valid value
 *                          for this hyperparameter;
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space; or if \p evaluation is not a
 *                              valid CCS evaluation
 * @return -#CCS_INVALID_EVALUATION if \p evaluation is not associated to the
 *                                  objective space; or if the number of values
 *                                  contained in \p evaluation is not equal to
 *                                  the number of hyperparameters in the
 *                                  features space
 */
extern ccs_result_t
ccs_objective_space_check_evaluation(ccs_objective_space_t  objective_space,
                                     ccs_evaluation_t       evaluation,
                                     ccs_bool_t            *is_valid_ret);

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
 *                          an hyperparameter value is not a valid value
 *                          for this hyperparameter;
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return -#CCS_INVALID_VALUE if \p values is NULL and num_values is greater
 *                             than 0
 * @return -#CCS_INVALID_EVALUATION if \p num_values is not equal to the number
 *                                  of hyperparameters in the objective space
 */
extern ccs_result_t
ccs_objective_space_check_evaluation_values(ccs_objective_space_t  objective_space,
                                            size_t                 num_values,
                                            ccs_datum_t           *values,
                                            ccs_bool_t            *is_valid_ret);

/**
 * Validate that a given value at the given index is valid in a objective
 * space, and return a sanitized value.
 * @param[in] objective_space
 * @param[in] index the index of the value in the objective_space
 * @param[in] value the datum to validate
 * @param[out] value_ret a pointer that will contain the validated value. If \p
 *                       value is a string \p value_ret will contain a non
 *                       transient string.
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return -#CCS_OUT_OF_BOUNDS if index is greater than the number of
 *                             hyperparameters in \p objective_space
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory while memoizing a
 *                             string
 * @return -#CCS_INVALID_VALUE if the value did not validate or if value_ret is
 *                             NULL
 */
extern ccs_result_t
ccs_objective_space_validate_value(ccs_objective_space_t  objective_space,
                                   size_t                 index,
                                   ccs_datum_t            value,
                                   ccs_datum_t           *value_ret);

/**
 * Add an objective to an objective space.
 * @param[in,out] objective_space
 * @param[in] expression the forbidden clause to dd to the configuration space
 * @param[in] type the type of the objective, either #CCS_MAXIMIZE or
 *                 #CCS_MINIMIZE
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space; or if \p expression is not
 *                              a valid CCS expression
 * @return -#CCS_INVALID_HYPERPARAMETER if expression references a
 *                                      hyperparameter that is not in the
 *                                      objective space
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate
 *                             internal data structures
 */
extern ccs_result_t
ccs_objective_space_add_objective(ccs_objective_space_t objective_space,
                                  ccs_expression_t      expression,
                                  ccs_objective_type_t  type);

/**
 * Add a list of objectives to an objective space.
 * @param[in,out] objective_space
 * @param[in] num_objectives the number of provided expressions
 * @param[in] expressions an array o \p num_objectives expressions to add as
 *                        objectives to the objective space
 * @param[in] types an array o \p num_objectives types of objectives
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space; or if at least one of the
 *                              provided expressions is not a valid CCS
 *                              expression
 * @return -#CCS_INVALID_VALUE if \p expressions is NULL and \p num_objectives
 *                             is greater than 0
 * @return -#CCS_INVALID_HYPERPARAMETER if an expression references a
 *                                      hyperparameter that is not in the
 *                                      objective space
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate
 *                             internal data structures
 */
extern ccs_result_t
ccs_objective_space_add_objectives(ccs_objective_space_t  objective_space,
                                   size_t                 num_objectives,
                                   ccs_expression_t      *expressions,
                                   ccs_objective_type_t  *types);

/**
 * Get the objective of rank index in a objective space.
 * @param[in] objective_space
 * @param[in] index the index of the objective to retrieve
 * @param[out] expression_ret a pointer to the variable that will contain the
 *                            returned expression
 * @param[out] type_ret a pointer to the variable that will contain the returned
 *                      objective type
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return -#CCS_INVALID_VALUE if \p expression_ret or \p type_ret are NULL
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the number of
 *                             objectives in the objective space
 */
extern ccs_result_t
ccs_objective_space_get_objective(ccs_objective_space_t  objective_space,
                                  size_t                 index,
                                  ccs_expression_t      *expression_ret,
                                  ccs_objective_type_t  *type_ret);

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
                     objective types
 * @param[out] num_objectives_ret a pointer to a variable that will contain the
 *                                 number of expressions that are or would be
 *                                 returned. Can be NULL
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p objective_space is not a valid CCS
 *                              objective space
 * @return -#CCS_INVALID_VALUE if \p expressions is NULL and \p num_objectives
 *                             is greater than 0; if \p types is NULL and \p
 *                             num_objectives is greater than 0; or if or if \p
 *                             expressions is NULL and \p num_objectives_ret is
 *                             NULL; or if \p num_objectives is less than then
 *                             number of expressions that would be returned
 */
extern ccs_result_t
ccs_objective_space_get_objectives(ccs_objective_space_t  objective_space,
                                   size_t                 num_objectives,
                                   ccs_expression_t      *expressions,
                                   ccs_objective_type_t  *types,
                                   size_t                *num_objectives_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_OBJECTIVE_SPACE
