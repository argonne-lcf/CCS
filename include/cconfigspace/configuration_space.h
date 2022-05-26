#ifndef _CCS_CONFIGURATION_SPACE
#define _CCS_CONFIGURATION_SPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file configuration_space.h
 * A configuration space is a context (see context.h) defining a set of
 * hyperparameters. Configuration space also offer as constraints system to
 * describe conditional hyperparameter activation as well as forbidden
 * hyperparameter configurations. Sampling distributions of hyperparameters can
 * be specified.
 */

/**
 * Create a new empty configuration space.
 * @param[in] name pointer to a string that will be copied internally
 * @param[in] user_data a pointer to the user data to attach to this
 *                      configuration space instance
 * @param[out] configuration_space_ret a pointer to the variable that will hold
 *                                     the newly created configuration space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_VALUE if \p name is NULL; or if \p
 *                             configuration_space_ret is NULL
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory to allocate the new
 *                             configuration space
 */
extern ccs_result_t
ccs_create_configuration_space(
	const char                *name,
	void                      *user_data,
	ccs_configuration_space_t *configuration_space_ret);

/**
 * Get the name of a configuration space.
 * @param[in] configuration_space
 * @param[out] name_ret a pointer to a `char *` variable which will contain a
 *                      pointer to the configuration space name.
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p name_ret is NULL
 */
extern ccs_result_t
ccs_configuration_space_get_name(
	ccs_configuration_space_t   configuration_space,
	const char                **name_ret);

/**
 * Set (replace) the internal rng of the configuration space.
 * @param[in,out] configuration_space
 * @param[in] rng the rng to use in the configuration space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space; or \p rng is not a valid
 *                              CCS rng
 */
extern ccs_result_t
ccs_configuration_space_set_rng(ccs_configuration_space_t configuration_space,
                                ccs_rng_t                 rng);

/**
 * Get the internal rng of the configuration space.
 * @param[in] configuration_space
 * @param[out] rng_ret a pointer to the variable that will contain the rng
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space; or \p rng_ret is NULL
 */
extern ccs_result_t
ccs_configuration_space_get_rng(ccs_configuration_space_t  configuration_space,
                                ccs_rng_t                 *rng_ret);

/**
 * Add a hyperparameter to the configuration space.
 * @param[in,out] configuration_space
 * @param[in] hyperparameter the hyperparameter to add to the configuration
 *                           space
 * @param[in] distribution optional, the 1 dimensional distribution to associate
 *                         to the hyperparameter. If NULL is passed, the default
 *                         distribution of the hyperparameter is used.
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space; or \p hyperparameter is not
 *                              a valid CCS hyperparameter; or if distribution
 *                              is given and distribution is not a valid CCS
 *                              distribution
 * @return -#CCS_INVALID_HYPERPARAMETER if \p hyperparameter's type is
 *                                      CCS_HYPERPARAMETER_TYPE_STRING; or if \p
 *                                      hyperparameter is already in the
 *                                      configuration space
 * @return -#CCS_INVALID_DISTRIBUTION if \p distribution has more than one
 *                                    dimension
 * @return -#CCS_OUT_OF_MEMORY if a memory could not be allocated to store
 *                             the additional hyperparameter and associated data
 *                             structures
 */
extern ccs_result_t
ccs_configuration_space_add_hyperparameter(
	ccs_configuration_space_t configuration_space,
	ccs_hyperparameter_t      hyperparameter,
	ccs_distribution_t        distribution);

/**
 * Add hyperparameters to the configuration space.
 * @param[in,out] configuration_space
 * @param[in] num_hyperparameters the number of provided hyperparameters
 * @param[in] hyperparameters an array of \p num_hyperparameters hyperparameters
 *                            to add to the configuration space
 * @param[in] distributions optional, an array of \p num_hyperparameters
 *                          distributions. If NULL, hyperparameter's default
 *                          distributions are used. If the array is provided
 *                          each distribution is optional, and NULL can be
 *                          provided to use the default distribution for a
 *                          specific hyperparameter
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space; or a hyperparameter is not
 *                              a valid CCS hyperparameter; or if a given
 *                              distribution is not a valid CCS distribution
 * @return -#CCS_INVALID_VALUE if \p hyperparameters is NULL and \p
 *                             num_hyperparameters is greater than 0
 * @return -#CCS_INVALID_HYPERPARAMETER if a hyperparameter's type is
 *                                      CCS_HYPERPARAMETER_TYPE_STRING; or if
 *                                      a hyperparameter is already in the
 *                                      configuration space; or if a
 *                                      hyperparameter with the same name
 *                                      already exists in the configuration
 *                                      space
 * @return -#CCS_INVALID_DISTRIBUTION if a distribution has more than one
 *                                    dimension
 * @return -#CCS_OUT_OF_MEMORY if memory could not be allocated to store
 *                             additional hyperparameters and associated data
 *                             structures
 */
extern ccs_result_t
ccs_configuration_space_add_hyperparameters(
	ccs_configuration_space_t  configuration_space,
	size_t                     num_hyperparameters,
	ccs_hyperparameter_t      *hyperparameters,
	ccs_distribution_t        *distributions);

/**
 * Set the distribution of one or more hyperparameters. Existing distributions
 * are discarded, and if a hyperparameter is left without a distribution it's
 * default distribution is used.
 * @param[in,out] configuration_space
 * @param[in] distribution the distribution to associate to the hyperparameters
 *                         at the indices given by \p indices
 * @param[in] indices an array of hyperparameters indices with as many elements
 *                    as the dimension of the distribution
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space; or distribution is not a
 *                              valid CCS distribution
 * @return -#CCS_INVALID_VALUE if \p indices is NULL; or if indices contains
 *                             values greater or equal to the number of
 *                             hyperparameters in the configuration space; or if
 *                             indices contain duplicate values
 * @return -#CCS_OUT_OF_MEMORY if a memory could not be allocated to store
 *                             additional hyperparameters and associated data
 *                             structures
 */
extern ccs_result_t
ccs_configuration_space_set_distribution(
	ccs_configuration_space_t  configuration_space,
	ccs_distribution_t         distribution,
	size_t                    *indices);

/**
 * Get the number of hyperparameters in a configuration space.
 * @param[in] configuration_space
 * @param[out] num_hyperparameters_ret a pointer to the variable that will
 *                                     contain the number of hyperparameters in
 *                                     the configuration space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p num_hyperparameters_ret is NULL
 */
extern ccs_result_t
ccs_configuration_space_get_num_hyperparameters(
	ccs_configuration_space_t  configuration_space,
	size_t                     *num_hyperparameters_ret);

/**
 * Get an hyperparameter in a configuration space given its index.
 * @param[in] configuration_space
 * @param[in] index the index of the hyperparameter to retrieve
 * @param[out] hyperparameter_ret a pointer to the variable that will contain
 *                                the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p hyperparameter_ret is NULL
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the count of
 *                             hyperparameters in the configuration space
 */
extern ccs_result_t
ccs_configuration_space_get_hyperparameter(
	ccs_configuration_space_t  configuration_space,
	size_t                     index,
	ccs_hyperparameter_t      *hyperparameter_ret);


/**
 * Get an hyperparameter's distribution in a configuration space given its
 * index.
 * @param[in] configuration_space
 * @param[in] index the index of the hyperparameter
 * @param[out] distribution_ret a pointer to the variable that will contain the
 *                              distribution
 * @param[out] index_ret a pointer to the variable that will contain the index
 *                       of the component in the distribution
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p distribution_ret is NULL; or if \p
 *                             index_ret is NULL
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the count of
 *                             hyperparameters in the configuration space
 */
extern ccs_result_t
ccs_configuration_space_get_hyperparameter_distribution(
	ccs_configuration_space_t  configuration_space,
	size_t                     index,
	ccs_distribution_t        *distribution_ret,
	size_t                    *index_ret);

/**
 * Get an hyperparameter in a configuration space given its name.
 * @param[in] configuration_space
 * @param[in] name the name of the hyperparameter to retrieve
 * @param[out] hyperparameter_ret a pointer to the variable that will contain
 *                                the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p name or \p hyperparameter_ret are NULL
 * @return -#CCS_INVALID_NAME if no hyperparameter with such \p name exist in
 *                            the \p configuration space
 */
extern ccs_result_t
ccs_configuration_space_get_hyperparameter_by_name(
		ccs_configuration_space_t  configuration_space,
		const char *               name,
		ccs_hyperparameter_t      *hyperparameter_ret);

/**
 * Get the index of an hyperparameter in the configuration space given its name.
 * @param[in] configuration_space
 * @param[in] name the name of the hyperparameter to retrieve the index of
 * @param[out] index_ret a pointer to the variable that will contain the index
 *                       of hyperparameter in the \p configuration_space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p name or \p index_ret are NULL
 * @return -#CCS_INVALID_NAME if no hyperparameter with such \p name exist in
 *                            the configuration space
 */
extern ccs_result_t
ccs_configuration_space_get_hyperparameter_index_by_name(
		ccs_configuration_space_t  configuration_space,
		const char                *name,
		size_t                    *index_ret);

/**
 * Get the index of an hyperparameter in the configuration space.
 * @param[in] configuration_space
 * @param[in] hyperparameter
 * @param[out] index_ret a pointer to the variable which will contain the index
 *                       of the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p index_ret is NULL
 * @return -#CCS_INVALID_HYPERPARAMETER if \p configuration_space does not
 *                                      contain \p hyperparameter
 */
extern ccs_result_t
ccs_configuration_space_get_hyperparameter_index(
		ccs_configuration_space_t  configuration_space,
		ccs_hyperparameter_t       hyperparameter,
		size_t                    *index_ret);

/**
 * Get the indices of a set of hyperparameters in a configuration space.
 * @param[in] configuration_space
 * @param[in] num_hyperparameters the number of hyperparameters to query the
 *                                index for
 * @param[in] hyperparameters an array of \p num_hyperparameters hyperparameters
 *                            to query the index for
 * @param[out] indexes an array of \p num_hyperparameters indices that will
 *                     contain the values of the hyperparamters indices
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p hyperparameters is NULL and \p
 *                             num_hyperparameters is greater than 0; or if \p
 *                             indexes is NULL and \p num_hyperparameters is
 *                             greater than 0
 * @return -#CCS_INVALID_HYPERPARAMETER if at least one of the hyperparameters
 *                                      is not contained in \p
 *                                      configuration_space
 */
extern ccs_result_t
ccs_configuration_space_get_hyperparameter_indexes(
		ccs_configuration_space_t  configuration_space,
		size_t                     num_hyperparameters,
		ccs_hyperparameter_t      *hyperparameters,
		size_t                    *indexes);

/**
 * Get the hyperparameters in the given configuration space.
 * @param[in] configuration_space
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
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p hyperparameters is NULL and \p
 *                             num_hyperparameters is greater than 0; or if \p
 *                             hyperparameters is NULL and
 *                             num_hyperparameters_ret is NULL; or if
 *                             \p num_hyperparameters is less than the number of
 *                             hyperparameters that would be returned
 */
extern ccs_result_t
ccs_configuration_space_get_hyperparameters(
	ccs_configuration_space_t  configuration_space,
	size_t                     num_hyperparameters,
	ccs_hyperparameter_t      *hyperparameters,
	size_t                    *num_hyperparameters_ret);

/**
 * Validate that a given value at the given index is valid in a configuration
 * space, and return a sanitized value.
 * @param[in] configuration_space
 * @param[in] index the index of the value in the configuration_space
 * @param[in] value the datum to validate
 * @param[out] value_ret a pointer that will contain the validated value. If \p
 *                       value is a string \p value_ret will contain a non
 *                       transient string.
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_OUT_OF_BOUNDS if index is greater than the number of
 *                             hyperparameters in \p configuration_space
 * @return -#CCS_OUT_OF_MEMORY if there was a lack of memory while memoizing a
 *                             string
 * @return -#CCS_INVALID_VALUE if the value did not validate or if value_ret is
 *                             NULL
 */
extern ccs_result_t
ccs_configuration_space_validate_value(
	ccs_configuration_space_t  configuration_space,
	size_t                     index,
	ccs_datum_t                value,
	ccs_datum_t               *value_ret);

/**
 * Set the active condition of a hyperparameter in a configuration space given
 * it's index.
 * @param[in, out] configuration_space
 * @param[in] hyperparameter_index the index of the hyperparameter to set the
 *                                 condition
 * @param[in] expression the condition to associate to the hyperparameter
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space; or if \p expression is not
 *                              a valid CCS expression
 * @return -#CCS_OUT_OF_BOUNDS if index is greater than the number of
 *                             hyperparameters in \p configuration_space
 * @return -#CCS_INVALID_HYPERPARAMETER if hyperparameter at index is already
 *                                      associated with a condition; or if the
 *                                      condition references a hyperparameter
 *                                      that is not in the configuration space
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to process the
 *                             dependency graph
 * @return -#CCS_INVALID_GRAPH if the addition of the condition would cause the
 *                             dependency graph to become invalid (cyclic)
 */
extern ccs_result_t
ccs_configuration_space_set_condition(
	ccs_configuration_space_t configuration_space,
	size_t                    hyperparameter_index,
	ccs_expression_t          expression);

/**
 * Get the active condition of a hyperparameter in a configuration space given
 * it's index.
 * @param[in] configuration_space
 * @param[in] hyperparameter_index the index of the hyperparameter to get the
 *                                 condition
 * @param[out] expression_ret a pointer to the variable that will contain the
 *                            expression, or NULL if the hyperparameter is not
 *                            associated with a condition
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_OUT_OF_BOUNDS if index is greater than the number of
 *                             hyperparameters in \p configuration_space
 * @return -#CCS_INVALID_VALUE if \p expression_ret is NULL
 */
extern ccs_result_t
ccs_configuration_space_get_condition(
	ccs_configuration_space_t  configuration_space,
	size_t                     hyperparameter_index,
	ccs_expression_t          *expression_ret);

/**
 * Get the active conditions of the hyperparameters in a configuration space.
 * @param[in] configuration_space
 * @param[in] num_expressions is the number of expressions that can be added to
 *                             \p expressions. If \p expressions is not NULL, \p
 *                             num_expressions must be greater than 0
 * @param[out] expressions an array of \p num_expressions that will contain the
 *                         returned expression, or NULL. If the array is too
 *                         big, extra values are set to NULL. If an
 *                         hyperparameter is not associated to an expression
 *                         NULL will be returned for this hyperparameter.
 * @param[out] num_expressions_ret a pointer to a variable that will contain
 *                                 the number of expression that are or would be
 *                                 returned. Can be NULL
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p expressions is NULL and \p num_expressions
 *                             is greater than 0; or if \p expressions is NULL
 *                             and num_expressions_ret is NULL; or if
 *                             num_expressions is is less than the number of
 *                             hyperparameters contained by configuration_space
 */
extern ccs_result_t
ccs_configuration_space_get_conditions(
	ccs_configuration_space_t  configuration_space,
	size_t                     num_expressions,
	ccs_expression_t          *expressions,
	size_t                    *num_expressions_ret);

/**
 * Add a forbidden clause to a configuration space.
 * @param[in,out] configuration_space
 * @param[in] expression the forbidden clause to dd to the configuration space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space; or if \p expression is not
 *                              a valid CCS expression
 * @return -#CCS_INVALID_HYPERPARAMETER if expression references a
 *                                      hyperparameter that is not in the
 *                                      configuration space
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate
 *                             internal data structures
 * @return -#CCS_INVALID_CONFIGURATION if adding the forbidden clause would
 *                                     render the default configuration invalid
 */
extern ccs_result_t
ccs_configuration_space_add_forbidden_clause(
	ccs_configuration_space_t configuration_space,
	ccs_expression_t          expression);

/**
 * Add a list of forbidden clauses to a configuration space.
 * @param[in,out] configuration_space
 * @param[in] num_expressions the number of provided expressions
 * @param[in] expressions an array o \p num_expressions expressions to add as
 *                        forbidden clauses to the configuration space
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space; or if at least one of the
 *                              provided expressions is not a valid CCS
 *                              expression
 * @return -#CCS_INVALID_VALUE if \p expressions is NULL and \p num_expressions
 *                             is greater than 0
 * @return -#CCS_INVALID_HYPERPARAMETER if an expression references a
 *                                      hyperparameter that is not in the
 *                                      configuration space
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate
 *                             internal data structures
 * @return -#CCS_INVALID_CONFIGURATION if adding one of the provided forbidden
 *                                     clause would render the default
 *                                     configuration invalid
 */
extern ccs_result_t
ccs_configuration_space_add_forbidden_clauses(
	ccs_configuration_space_t  configuration_space,
	size_t                     num_expressions,
	ccs_expression_t          *expressions);

/**
 * Get the forbidden clause of rank index in a configuration space.
 * @param[in] configuration_space
 * @param[in] index the index of the forbidden clause to retrieve
 * @param[out] expression_ret a pointer to the variable that will contain the
 *                            returned expression
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p expression_ret is NULL
 * @return -#CCS_OUT_OF_BOUNDS if \p index is greater than the number of
 *                             forbidden clauses in the configuration space
 */
extern ccs_result_t
ccs_configuration_space_get_forbidden_clause(
	ccs_configuration_space_t  configuration_space,
	size_t                     index,
	ccs_expression_t          *expression_ret);

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
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p expressions is NULL and \p num_expressions
 *                             is greater than 0; or if or if \p expressions is
 *                             NULL and \p num_expressions_ret is NULL; or if \p
 *                             num_expressions is less than then number of
 *                             expressions that would be returned
 */
extern ccs_result_t
ccs_configuration_space_get_forbidden_clauses(
	ccs_configuration_space_t  configuration_space,
	size_t                     num_expressions,
	ccs_expression_t          *expressions,
	size_t                    *num_expressions_ret);

/**
 * Check that a configuration is a valid in a configuration space.
 * @param[in] configuration_space
 * @param[in] configuration
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space; or if \p configuration is
 *                              not a valid CCS configuration
 * @return -#CCS_INVALID_CONFIGURATION if \p configuration is not associated to
 *                                     the configuration space; or if the number
 *                                     of values contained in \p configuration
 *                                     is not equal to the number of
 *                                     hyperparameters in the configuration
 *                                     space; or if an active hyperparameter
 *                                     value is not a valid value for this
 *                                     hyperparameter; or if a forbidden clause
 *                                     would be evaluating to #ccs_true
 */
extern ccs_result_t
ccs_configuration_space_check_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t       configuration);

/**
 * Check that a set of values would create a valid configuration for a
 * configuration space.
 * @param[in] configuration_space
 * @param[in] num_values the number of provided values
 * @param[in] values an array of \p num_values values that would become a
 *                   configuration
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p values is NULL and num_values is greater
 *                             than 0
 * @return -#CCS_INVALID_CONFIGURATION if \p num_values is not equal to the
 *                                     number of hyperparameters in the
 *                                     configuration space; or if an active
 *                                     hyperparameter value is not a valid value
 *                                     for this hyperparameter; or if a
 *                                     forbidden clause would be evaluating to
 *                                     #ccs_true
 */
extern ccs_result_t
ccs_configuration_space_check_configuration_values(
	ccs_configuration_space_t  configuration_space,
	size_t                     num_values,
	ccs_datum_t               *values);

/**
 * Get the default configuration of a configuration space
 * @param[in] configuration_space
 * @param[out] configuration_ret a pointer to the variable that will contain the
 *                               returned default configuration
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if configuration_ret is NULL
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new configuration
 */
extern ccs_result_t
ccs_configuration_space_get_default_configuration(
	ccs_configuration_space_t  configuration_space,
	ccs_configuration_t       *configuration_ret);

/**
 * Get a configuration sampled randomly from a configuration space.
 * Hyperparameters that were not specified distributions are sampled according
 * to their default distribution. Hyperparameter that are found to be inactive
 * will have the #ccs_inactive value. Returned configuration is valid.
 * @param[in] configuration_space
 * @param[out] configuration_ret a pointer to the variable that will contain the
 *                               returned configuration
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if configuration_ret is NULL
 * @return -#CCS_SAMPLING_UNSUCCESSFUL if no valid configuration could be
 *                                     sampled
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new configuration
 */
extern ccs_result_t
ccs_configuration_space_sample(ccs_configuration_space_t  configuration_space,
                               ccs_configuration_t       *configuration_ret);

/**
 * Get a given number of configurations sampled randomly from a configuration
 * space. Hyperparameters that were not specified distributions are sampled
 * according to their default distribution. Hyperparameter that are found to be
 * inactive will have the #ccs_inactive value. Returned configurations are
 * valid.
 * @param[in] configuration_space
 * @param[in] num_configurations the number of requested configurations
 * @param[out] configurations an array of \p num_configurations that will
 *                            contain the requested configurations
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p configurations is NULL and \p
 *                             num_configurations is greater than 0
 * @return -#CCS_SAMPLING_UNSUCCESSFUL if no or not enough valid configurations
 *                                     could be sampled. Configurations that
 *                                     could be sampled will be returned
 *                                     contiguously, and the rest will be NULL
 * @return -#CCS_OUT_OF_MEMORY if there was not enough memory to allocate new
 *                             configurations. Configurations that could be
 *                             allocated will be returned, and the rest will be
 *                             NULL
 */
extern ccs_result_t
ccs_configuration_space_samples(ccs_configuration_space_t  configuration_space,
                                size_t                     num_configurations,
                                ccs_configuration_t       *configurations);

#ifdef __cplusplus
}
#endif

#endif //_CCS_CONFIGURATION_SPACE
