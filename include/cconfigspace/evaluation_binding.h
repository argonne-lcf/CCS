#ifndef _CCS_EVALUATION_BINDING_H
#define _CCS_EVALUATION_BINDING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file evaluation_binding.h An evaluation binding is a binding (see
 * binding.h) over an objective space (see objective_space.h). Successful
 * evaluations over the same objective space are weakly ordered by their
 * objective values. Evaluation that have failed must report a result code
 * different than CCS_RESULT_SUCCESS.
 */

/**
 * The different possible return codes when comparing two evaluations.
 */
enum ccs_comparison_e {
	/** The first configuration is better then the second */
	CCS_COMPARISON_BETTER         = -1,
	/** The two configurations are equivalent */
	CCS_COMPARISON_EQUIVALENT     = 0,
	/** The first configuration is worse than the second */
	CCS_COMPARISON_WORSE          = 1,
	/** The two configurations cannot be compared */
	CCS_COMPARISON_NOT_COMPARABLE = 2,
	/** Guard */
	CCS_COMPARISON_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_COMPARISON_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS the result of comparing evaluations.
 */
typedef enum ccs_comparison_e ccs_comparison_t;

/**
 * Get the objective space associated with an evaluation binding.
 * @param[in] evaluation_binding
 * @param[out] objective_space_ret a pointer to the variable that will
 *                                 contain the objective space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p evaluation_binding is not a
 * valid CCS evaluation binding
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p objective_space_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_evaluation_binding_get_objective_space(
	ccs_evaluation_binding_t evaluation,
	ccs_objective_space_t   *objective_space_ret);

/**
 * Get the result code associated with an evaluation.
 * @param[in] evaluation
 * @param[out] result_ret a pointer to the variable that will contain the
 *                        returned result code
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p evaluation is not a valid CCS
 * evaluation
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p result_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_evaluation_binding_get_result(
	ccs_evaluation_binding_t evaluation,
	ccs_evaluation_result_t *result_ret);

/**
 * Get the value of an objective for a valid evaluation in the context of its
 * objective space.
 * @param[in] evaluation
 * @param[in] index the index of the objective in the objective space
 * @param[out] value_ret a pointer to the variable that will contain the value
 *                       of the objective
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p evaluation is not a valid CCS
 * evaluation
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p value_ret is NULL; or if there
 * was an issue evaluating the objective
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if \p index is greater than the
 * number of objective in the objective space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_evaluation_binding_get_objective_value(
	ccs_evaluation_binding_t evaluation,
	size_t                   index,
	ccs_datum_t             *value_ret);

/**
 * Get the values of the objectives for a valid evaluation in the context of its
 * objective space.
 * @param[in] evaluation
 * @param[in] num_values the number of values that \p values can contain
 * @param[out] values an optional array of values that will contain the returned
 *                    objective values. If values is bigger than the number of
 *                    objectives, extra values will be set to
 *                    #CCS_DATA_TYPE_NONE
 * @param[out] num_values_ret an optional pointer to a variable that will
 *                            contain the number of values that are or would be
 *                            returned. Can be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p evaluation is not a valid CCS
 * evaluation binding
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p values is NULL and \p
 * num_values is greater than 0; or if values is NULL and num_values_ret is
 * NULL; or if there was an issue evaluating any of the objectives
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_evaluation_binding_get_objective_values(
	ccs_evaluation_binding_t evaluation,
	size_t                   num_values,
	ccs_datum_t             *values,
	size_t                  *num_values_ret);

/**
 * Check that an evaluation values are valid in the objective space.
 * @param[in] evaluation
 * @param[out] is_valid_ret a pointer to a variable that will hold the result
 *                          of the check. Result will be #CCS_TRUE if the
 *                          evaluation is valid. Result will be #CCS_FALSE if
 *                          an parameter value is not a valid value
 *                          for this parameter
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p evaluation is not a valid CCS
 * evaluation
 * @return #CCS_RESULT_ERROR_INVALID_EVALUATION if \p evaluation was found to be
 * invalid in the context of the objective space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_evaluation_binding_check(
	ccs_evaluation_binding_t evaluation,
	ccs_bool_t              *is_valid_ret);

/**
 * Compare two successful evaluations objectives.
 * @param[in] evaluation the first evaluation
 * @param[in] other_evaluation the second evaluation
 * @param[out] result_ret a pointer to the variable that will contain the result
 *                        of the comparison. Will contain
 *                        #CCS_COMPARISON_BETTER, #CCS_COMPARISON_EQUIVALENT,
 *                        #CCS_COMPARISON_WORSE, or
 *                        #CCS_COMPARISON_NOT_COMPARABLE if the first
 *                        evaluation is found to be respectively better,
 *                        equivalent, worse, or not comparable with the second
 *                        evaluation.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p evaluation or \p
 * other_evaluation are not valid CCS evaluations; or if \p evaluation and \p
 * other_evaluation do not share the same objective space; or if any of the
 * the evaluation is associated a result code different than #CCS_RESULT_SUCESS;
 * or if both evaluations are not ot the same type
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p result_ret is NULL; or if there
 * was an issue evaluating any of the objectives
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_evaluation_binding_compare(
	ccs_evaluation_binding_t evaluation,
	ccs_evaluation_binding_t other_evaluation,
	ccs_comparison_t        *result_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_EVALUATION_BINDING_H
