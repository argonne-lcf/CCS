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
 * @param[in] configuration space
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
 * Get the associated `user_data` pointer of a configuration space.
 * @param[in] configuration_space
 * @param[out] user_data_ret a pointer to `void *` variable that will contain
 *                           the value of the `user_data`
 * @return #CCS_SUCCESS on success
 * @return -#CCS_INVALID_OBJECT if \p configuration_space is not a valid CCS
 *                              configuration space
 * @return -#CCS_INVALID_VALUE if \p user_data_ret is NULL
 */
extern ccs_result_t
ccs_configuration_space_get_user_data(
	ccs_configuration_space_t   configuration_space,
	void                      **user_data_ret);

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
 *                                      CCS_HYPERPARAMETER_TYPE_STRING; or if \p
 *                                      a hyperparameter is already in the
 *                                      configuration space
 * @return -#CCS_INVALID_DISTRIBUTION if a distribution has more than one
 *                                    dimension
 * @return -#CCS_OUT_OF_MEMORY if a memory could not be allocated to store
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

extern ccs_result_t
ccs_configuration_space_get_hyperparameter(
	ccs_configuration_space_t  configuration_space,
	size_t                     index,
	ccs_hyperparameter_t      *hyperparameter_ret);

extern ccs_result_t
ccs_configuration_space_get_hyperparameter_distribution(
	ccs_configuration_space_t  configuration_space,
	size_t                     index,
	ccs_distribution_t        *distribution_ret,
	size_t                    *index_ret);

extern ccs_result_t
ccs_configuration_space_get_hyperparameter_by_name(
		ccs_configuration_space_t  configuration_space,
		const char *               name,
		ccs_hyperparameter_t      *hyperparameter_ret);

extern ccs_result_t
ccs_configuration_space_get_hyperparameter_index_by_name(
		ccs_configuration_space_t  configuration_space,
		const char                *name,
		size_t                    *index_ret);

extern ccs_result_t
ccs_configuration_space_get_hyperparameter_index(
		ccs_configuration_space_t  configuration_space,
		ccs_hyperparameter_t       hyperparameter,
		size_t                    *index_ret);

extern ccs_result_t
ccs_configuration_space_get_hyperparameter_indexes(
		ccs_configuration_space_t  configuration_space,
		size_t                     num_hyperparameters,
		ccs_hyperparameter_t      *hyperparameters,
		size_t                    *indexes);

extern ccs_result_t
ccs_configuration_space_get_hyperparameters(
	ccs_configuration_space_t  configuration_space,
	size_t                     num_hyperparameters,
	ccs_hyperparameter_t      *hyperparameters,
	size_t                    *num_hyperparameters_ret);

extern ccs_result_t
ccs_configuration_space_validate_value(
	ccs_configuration_space_t  configuration_space,
	size_t                     index,
	ccs_datum_t                value,
	ccs_datum_t               *value_ret);

extern ccs_result_t
ccs_configuration_space_set_condition(
	ccs_configuration_space_t configuration_space,
	size_t                    hyperparameter_index,
	ccs_expression_t          expression);

extern ccs_result_t
ccs_configuration_space_get_condition(
	ccs_configuration_space_t  configuration_space,
	size_t                     hyperparameter_index,
	ccs_expression_t          *expression_ret);

extern ccs_result_t
ccs_configuration_space_get_conditions(
	ccs_configuration_space_t  configuration_space,
	size_t                     num_expressions,
	ccs_expression_t          *expressions,
	size_t                    *num_expressions_ret);

extern ccs_result_t
ccs_configuration_space_add_forbidden_clause(
	ccs_configuration_space_t configuration_space,
	ccs_expression_t          expression);

extern ccs_result_t
ccs_configuration_space_add_forbidden_clauses(
	ccs_configuration_space_t  configuration_space,
	size_t                     num_expressions,
	ccs_expression_t          *expressions);

extern ccs_result_t
ccs_configuration_space_get_forbidden_clause(
	ccs_configuration_space_t  configuration_space,
	size_t                     index,
	ccs_expression_t          *expression_ret);

extern ccs_result_t
ccs_configuration_space_get_forbidden_clauses(
	ccs_configuration_space_t  configuration_space,
	size_t                     num_expressions,
	ccs_expression_t          *expressions,
	size_t                    *num_expressions_ret);

//   Configuration related functions
extern ccs_result_t
ccs_configuration_space_check_configuration(
	ccs_configuration_space_t configuration_space,
	ccs_configuration_t       configuration);

extern ccs_result_t
ccs_configuration_space_check_configuration_values(
	ccs_configuration_space_t  configuration_space,
	size_t                     num_values,
	ccs_datum_t               *values);

extern ccs_result_t
ccs_configuration_space_get_default_configuration(
	ccs_configuration_space_t  configuration_space,
	ccs_configuration_t       *configuration_ret);

extern ccs_result_t
ccs_configuration_space_sample(ccs_configuration_space_t  configuration_space,
                               ccs_configuration_t       *configuration_ret);

extern ccs_result_t
ccs_configuration_space_samples(ccs_configuration_space_t  configuration_space,
                                size_t                     num_configurations,
                                ccs_configuration_t       *configurations);

#ifdef __cplusplus
}
#endif

#endif //_CCS_CONFIGURATION_SPACE
