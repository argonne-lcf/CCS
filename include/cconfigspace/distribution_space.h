#ifndef _CCS_DISTRIBUTION_SPACE
#define _CCS_DISTRIBUTION_SPACE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file distribution_space.h
 * A distribution space is set of sampling distribution over the parameters of
 * a configuration space.
 */

/**
 * Create an new distribution space.
 * @param[in] configuration_space the configuration space the distribution
 *                                space will be used to sample
 * @param[out] distribution_space_ret a pointer to the variable that will
 *                                    contain the newly created distribution
 *                                    space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p configuration_space is not a
 * valid CCS configuration space; or if \p objective_space is not a valid CCS
 * objective space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p distribution_space_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_create_distribution_space(
	ccs_configuration_space_t configuration_space,
	ccs_distribution_space_t *distribution_space_ret);

/**
 * Get the associated configuration space.
 * @param[in] distribution_space
 * @param[out] configuration_space_ret a pointer to the variable that will
 *                                     contain the configuration space
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p distribution_space is not a
 * valid CCS distribution space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p configuration_space_ret is NULL
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_distribution_space_get_configuration_space(
	ccs_distribution_space_t   distribution_space,
	ccs_configuration_space_t *configuration_space_ret);

/**
 * Set the distribution of one or more parameters. Existing distributions
 * are discarded, and if a parameter is left without a distribution it's
 * default distribution is used.
 * @param[in,out] distribution_space
 * @param[in] distribution the distribution to associate to the parameters
 *                         at the indices given by \p indices
 * @param[in] indices an array of parameters indices with as many elements
 *                    as the dimension of the distribution
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p distribution_space is not a
 * valid CCS distribution space; or distribution is not a valid CCS
 * distribution
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p indices is NULL; or if indices
 * contains values greater or equal to the number of parameters in the
 * distribution space's configuration space; or if indices contain duplicate
 * values
 * @return #CCS_RESULT_ERROR_OUT_OF_MEMORY if a memory could not be allocated to
 * store additional parameters and associated data structures
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_distribution_space_set_distribution(
	ccs_distribution_space_t distribution_space,
	ccs_distribution_t       distribution,
	size_t                  *indices);

/**
 * Get a parameter's distribution in a distribution space given its
 * index.
 * @param[in] distribution_space
 * @param[in] index the index of the parameter
 * @param[out] distribution_ret a pointer to the variable that will contain the
 *                              distribution
 * @param[out] index_ret a pointer to the variable that will contain the index
 *                       of the component in the distribution
 * @return #CCS_RESULT_SUCCESS on success
 * @return #CCS_RESULT_ERROR_INVALID_OBJECT if \p distribution_space is not a
 * valid CCS distribution space
 * @return #CCS_RESULT_ERROR_INVALID_VALUE if \p distribution_ret is NULL; or if
 * \p index_ret is NULL
 * @return #CCS_RESULT_ERROR_OUT_OF_BOUNDS if \p index is greater than the count
 * of parameters in the distribution space's configuration space
 * @remarks
 *   This function is thread-safe
 */
extern ccs_result_t
ccs_distribution_space_get_parameter_distribution(
	ccs_distribution_space_t distribution_space,
	size_t                   index,
	ccs_distribution_t      *distribution_ret,
	size_t                  *index_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_DISTRIBUTION_SPACE
