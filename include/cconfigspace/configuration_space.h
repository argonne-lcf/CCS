#ifndef _CCS_CONFIGURATION_SPACE
#define _CCS_CONFIGURATION_SPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file configuration_space.h
 * A configuration space is a context (see context.h) defining a set of
 * parameters. Configuration space also offer as constraints system to
 * describe conditional parameter activation as well as forbidden
 * parameter configurations.
 */

/**
 * Create a new configuration space.
 * @param[in] name pointer to a string that will be copied internally
 * @param[in] num_parameters the number of provided parameters
 * @param[in] parameters an array of \p num_parameters parameters
 *                       to add to the configuration space
 * @param[in] conditions an optional array of \p num_parameters expressions
 *                       setting the active condition of respective parameters.
 *                       a NULL entry in the array means no condition is
 *                       attached to the corresponding parameter.
 * @param[in] num_forbidden_clauses the number of provided forbidden clauses
 * @param[in] forbidden_clauses an array o \p num_forbidden_clauses expressions
 *                              to add as forbidden clauses to the
 *                              configuration space
 * @param[out] configuration_space_ret a pointer to the variable that will hold
 *                                     the newly created configuration space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p name is NULL; or if \p
 * configuration_space_ret is NULL; or if \p parameters is NULL; or if \p
 * num_parameters is NULL; or if \p forbidden_clauses is NULL and \p
 * num_forbidden_clauses is greater than 0
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if a parameter is not a valid CCS
 * parameter; or if an expression is not a valid CCS expression
 * @return #CCS_RESULT_ERROR_INVALID_PARAMETER if a parameter's type is
 * CCS_PARAMETER_TYPE_STRING; or if a parameter appears more than once in \p
 * parameters; or if two or more parameters share the same name; or if an
 * expression references a parameter that is not in the configuration space
 * @return #CCS_RESULT_ERROR_INVALID_CONFIGURATION if adding one of the provided
 * forbidden clause would render the default configuration invalid
 * @return #CCS_RESULT_ERROR_INVALID_GRAPH if the addition of the conditions
 * would cause the dependency graph to become invalid (cyclic)
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was a lack of memory to
 * allocate the new configuration space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_configuration_space(
	const char                *name,
	size_t                     num_parameters,
	ccs_parameter_t           *parameters,
	ccs_expression_t          *conditions,
	size_t                     num_forbidden_clauses,
	ccs_expression_t          *forbidden_clauses,
	ccs_configuration_space_t *configuration_space_ret);

/**
 * Set (replace) the internal rng of the configuration space.
 * @param[in,out] configuration_space
 * @param[in] rng the rng to use in the configuration space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space; or \p rng is not a valid CCS rng
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_configuration_space_set_rng(
	ccs_configuration_space_t configuration_space,
	ccs_rng_t                 rng);

/**
 * Get the internal rng of the configuration space.
 * @param[in] configuration_space
 * @param[out] rng_ret a pointer to the variable that will contain the rng
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p rng_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_configuration_space_get_rng(
	ccs_configuration_space_t configuration_space,
	ccs_rng_t                *rng_ret);

/**
 * Get the active condition of a parameter in a configuration space given
 * it's index.
 * @param[in] configuration_space
 * @param[in] parameter_index the index of the parameter to get the condition
 * @param[out] expression_ret a pointer to the variable that will contain the
 *                            expression, or NULL if the parameter is not
 *                            associated with a condition
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if index is greater than the number
 * of parameters in \p configuration_space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p expression_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_configuration_space_get_condition(
	ccs_configuration_space_t configuration_space,
	size_t                    parameter_index,
	ccs_expression_t         *expression_ret);

/**
 * Get the active conditions of the parameters in a configuration space.
 * @param[in] configuration_space
 * @param[in] num_expressions is the number of expressions that can be added to
 *                            \p expressions. If \p expressions is not NULL, \p
 *                            num_expressions must be greater than 0
 * @param[out] expressions an array of \p num_expressions that will contain the
 *                         returned expression, or NULL. If the array is too
 *                         big, extra values are set to NULL. If an
 *                         parameter is not associated to an expression
 *                         NULL will be returned for this parameter.
 * @param[out] num_expressions_ret a pointer to a variable that will contain
 *                                 the number of expression that are or would be
 *                                 returned. Can be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p expressions is NULL and \p
 * num_expressions is greater than 0; or if \p expressions is NULL and
 * num_expressions_ret is NULL; or if num_expressions is is less than the number
 * of parameters contained by configuration_space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_configuration_space_get_conditions(
	ccs_configuration_space_t configuration_space,
	size_t                    num_expressions,
	ccs_expression_t         *expressions,
	size_t                   *num_expressions_ret);

/**
 * Get the forbidden clause of rank index in a configuration space.
 * @param[in] configuration_space
 * @param[in] index the index of the forbidden clause to retrieve
 * @param[out] expression_ret a pointer to the variable that will contain the
 *                            returned expression
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p expression_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if \p index is greater than the
 * number of forbidden clauses in the configuration space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_configuration_space_get_forbidden_clause(
	ccs_configuration_space_t configuration_space,
	size_t                    index,
	ccs_expression_t         *expression_ret);

/**
 * Get the forbidden clauses in a configuration space.
 * @param[in] configuration_space
 * @param[in] num_expressions the number of expressions that can be added to \p
 *                            expressions. If \p expressions is not NULL, \p
 *                            num_expressions must be greater than 0
 * @param[out] expressions an array of \p num_expressions that will contain the
 *                         returned expressions, or NULL. If the array is too
 *                         big, extra values are set to NULL
 * @param[out] num_expressions_ret a pointer to a variable that will contain the
 *                                 number of expressions that are or would be
 *                                 returned. Can be NULL
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p expressions is NULL and \p
 * num_expressions is greater than 0; or if or if \p expressions is NULL and \p
 * num_expressions_ret is NULL; or if \p num_expressions is less than then
 * number of expressions that would be returned
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_configuration_space_get_forbidden_clauses(
	ccs_configuration_space_t configuration_space,
	size_t                    num_expressions,
	ccs_expression_t         *expressions,
	size_t                   *num_expressions_ret);

/**
 * Check that a configuration is a valid in a configuration space.
 * @param[in] configuration_space
 * @param[in] configuration
 * @param[out] is_valid_ret a pointer to a variable that will hold the result
 *                          of the check. Result will be #CCS_TRUE if the
 *                          configuration is valid. Result will be #CCS_FALSE
 *                          if an active parameter value is not a valid value
 *                          for this parameter; or if an inactive parameter
 *                          value is not inactive; or if a forbidden clause
 *                          would be evaluating to #ccs_true
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space; or if \p configuration is not a valid CCS
 * configuration
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p is_valid_ret is NULL
 * @return #CCS_RESULT_ERROR_INVALID_CONFIGURATION if \p configuration is not
 * associated to the configuration space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_configuration_space_check_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t       configuration,
	ccs_bool_t               *is_valid_ret);

/**
 * Get the default configuration of a configuration space
 * @param[in] configuration_space
 * @param[out] configuration_ret a pointer to the variable that will contain the
 *                               returned default configuration
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if configuration_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate the new configuration
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_configuration_space_get_default_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t      *configuration_ret);

/**
 * Get a configuration sampled randomly from a configuration space.
 * Parameters that were not specified distributions are sampled according
 * to their default distribution. Parameter that are found to be inactive
 * will have the #ccs_inactive value. Returned configuration is valid.
 * @param[in] configuration_space
 * @param[in] distribution_space an optional distribution space to use
 * @param[in] rng an optional rng to use
 * @param[out] configuration_ret a pointer to the variable that will contain the
 *                               returned configuration
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space; or if \p distribution_space is not a valid
 * CCS distribution space; or if \p rng is not a valid CCS rng
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if configuration_ret is NULL
 * @return #CCS_RESULT_ERROR_SAMPLING_UNSUCCESSFUL if no valid configuration
 * could be sampled
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate the new configuration
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_configuration_space_sample(
	ccs_configuration_space_t configuration_space,
	ccs_distribution_space_t  distribution_space,
	ccs_rng_t                 rng,
	ccs_configuration_t      *configuration_ret);

/**
 * Get a given number of configurations sampled randomly from a configuration
 * space. Parameters that were not specified distributions are sampled
 * according to their default distribution. Parameter that are found to be
 * inactive will have the #ccs_inactive value. Returned configurations are
 * valid.
 * @param[in] configuration_space
 * @param[in] distribution_space an optional distribution space to use
 * @param[in] rng an optional rng to use
 * @param[in] num_configurations the number of requested configurations
 * @param[out] configurations an array of \p num_configurations that will
 *                            contain the requested configurations
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space; or if \p distribution_space is not a valid
 * CCS distribution space; or if \p rng is not a valid CCS rng
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configurations is NULL and \p
 * num_configurations is greater than 0
 * @return #CCS_RESULT_ERROR_SAMPLING_UNSUCCESSFUL if no or not enough valid
 * configurations could be sampled. Configurations that could be sampled will be
 * returned contiguously, and the rest will be NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if there was not enough memory to
 * allocate new configurations. Configurations that could be allocated will be
 * returned, and the rest will be NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_configuration_space_samples(
	ccs_configuration_space_t configuration_space,
	ccs_distribution_space_t  distribution_space,
	ccs_rng_t                 rng,
	size_t                    num_configurations,
	ccs_configuration_t      *configurations);

#ifdef __cplusplus
}
#endif

#endif //_CCS_CONFIGURATION_SPACE
