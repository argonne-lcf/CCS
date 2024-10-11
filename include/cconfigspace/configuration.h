#ifndef _CCS_CONFIGURATION_H
#define _CCS_CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file configuration.h
 * A configuration is a binding (see binding.h) on a configuration space (see
 * configuration_space.h). Configurations are immutable except from a reference
 * counting and callback management point of view.
 */

/**
 * Create a new instance of a configuration on a given configuration space. If
 * no values are provided the values will be initialized to #CCS_DATA_TYPE_NONE.
 * @param[in] configuration_space
 * @param[in] features an optional features to use. If NULL and a feature space
 *                     was provided at \p configuration_space creation, the
 *                     deafult features of the feature space will be used.
 * @param[in] num_values the number of provided values to initialize the
 *            configuration
 * @param[in] values an optional array of values to initialize the configuration
 * @param[out] configuration_ret a pointer to the variable that will hold the
 *             newly created configuration
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space; or if \p features is not NULL and is not a
 * valid CCS features
 * @return #CCS_RESULT_ERROR_INVALID_FEATURES if features feature space is not
 * the same as the feature space provided at \p configuration_space creation.
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configuration_ret is NULL; or
 * if \p values is NULL and \p num_values is greater than 0; or if the number of
 * values provided is not equal to the number of parameters in the configuration
 * space
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new configuration
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_features_t            features,
	size_t                    num_values,
	ccs_datum_t              *values,
	ccs_configuration_t      *configuration_ret);

/**
 * Get the associated configuration space.
 * @param[in] configuration
 * @param[out] configuration_space_ret a pointer to the variable that will
 *                                     contain the configuration space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration is not a valid
 * CCS configuration
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configuration_space_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_configuration_get_configuration_space(
	ccs_configuration_t        configuration,
	ccs_configuration_space_t *configuration_space_ret);

/**
 * Get the associated features.
 * @param[in] configuration
 * @param[out] features_ret a pointer to the variable that will contain the
 *                          returned features or NULL if none is associated
 *                          to the configuration.
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration is not a valid
 * CCS configuration
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p features_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_configuration_get_features(
	ccs_configuration_t configuration,
	ccs_features_t     *features_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_CONFIGURATION_H
