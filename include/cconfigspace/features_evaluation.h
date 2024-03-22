#ifndef _CCS_FEATURES_EVALUATION_H
#define _CCS_FEATURES_EVALUATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file features_evaluation.h
 * A features evaluation is an evaluation binding (see evaluation_binding.h)
 * configuration (see configuration.h) and features (see features.h).
 */

/**
 * Create a new instance of a features evaluation on a given objective_space for
 * a given configuration and features.
 * @param[in] objective_space the objective space to associate with the
 *                            evaluation
 * @param[in] configuration the configuration to associate with the evaluation
 * @param[in] features the features to associate with the evaluation
 * @param[in] result the result code associated with the evaluation
 * @param[in] num_values the number of provided values to initialize the
 *                       evaluation
 * @param[in] values an optional array of values to initialize the evaluation
 * @param[out] features_evaluation_ret a pointer to the variable that will hold
 *                                     the newly created evaluation
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p objective_space is not a valid
 * CCS objective space; or if \p configuration is not a valid CCS configuration;
 * or the features is not a valid CCS features
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p features evaluation_ret is
 * NULL; or if \p values is NULL and \p num_values is greater than 0; or if the
 * number of values provided is not equal to the number of parameters in the
 * objective space
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new evaluation
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_features_evaluation(
	ccs_objective_space_t      objective_space,
	ccs_configuration_t        configuration,
	ccs_features_t             features,
	ccs_evaluation_result_t    result,
	size_t                     num_values,
	ccs_datum_t               *values,
	ccs_features_evaluation_t *features_evaluation_ret);

/**
 * Get the configuration associated with a features evaluation.
 * @param[in] features_evaluation
 * @param[out] configuration_ret a pointer to the variable that will contain
 *                               the configuration
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_evaluation is not a
 * valid CCS features_evaluation
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configuration_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_evaluation_get_configuration(
	ccs_features_evaluation_t features_evaluation,
	ccs_configuration_t      *configuration_ret);

/**
 * Get the features associated with a features evaluation.
 * @param[in] features_evaluation
 * @param[out] features_ret a pointer to the variable that will contain
 *                          the features
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p features_evaluation is not a
 * valid CCS features_evaluation
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p features_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_features_evaluation_get_features(
	ccs_features_evaluation_t features_evaluation,
	ccs_features_t           *features_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_FEATURES_EVALUATION_H
