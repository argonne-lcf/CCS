#ifndef _CCS_TREE_EVALUATION_H
#define _CCS_TREE_EVALUATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file tree_evaluation.h A tree evaluation is an evaluation binding
 * (see evaluation_binding.h) given a specific tree configuration
 * (see tree_configuration.h).
 */

/**
 * Create a new instance of a tree evaluation on a given objective space for a
 * given tree configuration.
 * @param[in] objective_space the objective space to associate with the
 *                            evaluation
 * @param[in] configuration the configuration to associate with the evaluation
 * @param[in] result the result code associated with the evaluation
 * @param[in] num_values the number of provided values to initialize the
 *                       evaluation
 * @param[in] values an optional array of values to initialize the evaluation
 * @param[out] evaluation_ret a pointer to the variable that will hold the
 *             newly created evaluation
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid
 * CCS objective space; or if \p configuration is not a valid CCS tree
 * configuration
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p evaluation_ret is NULL; or if
 * \p values is NULL and \p num_values is greater than 0; or if the number of
 * values provided is not equal to the number of parameters in the objective
 * space
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new evaluation
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_tree_evaluation(
	ccs_objective_space_t    objective_space,
	ccs_tree_configuration_t configuration,
	ccs_evaluation_result_t  result,
	size_t                   num_values,
	ccs_datum_t             *values,
	ccs_tree_evaluation_t   *evaluation_ret);

/**
 * Get the configuration associated with a tree evaluation.
 * @param[in] evaluation
 * @param[out] configuration_ret a pointer to the variable that will contain
 *                               the configuration
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p evaluation is not a valid CCS
 * tree evaluation
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configuration_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_tree_evaluation_get_configuration(
	ccs_tree_evaluation_t     evaluation,
	ccs_tree_configuration_t *configuration_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_EVALUATION_H
