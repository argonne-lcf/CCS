#ifndef _CCS_DISTRIBUTION_H
#define _CCS_DISTRIBUTION_H

/**
 * @file distribution.h
 * A Distribution is the probability distribution of a random variable. CCS
 * supports discrete and contiguous random variables. CCS also supports
 * composing distributions to create mixture distributions or multivariate
 * distributions.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CCS distribution types.
 */
enum ccs_distribution_type_e {
	/** A uniform distribution over floating point or integer values */
	CCS_UNIFORM,
	/** A normal distribution over floating point or integer values */
	CCS_NORMAL,
	/** A roulette wheel selection distribution */
	CCS_ROULETTE,
	/** A mixture distribution */
	CCS_MIXTURE,
	/** A multivariate distribution */
	CCS_MULTIVARIATE,
	/** Guard */
	CCS_DISTRIBUTION_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_DISTRIBUTION_TYPE_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS distribution type.
 */
typedef enum ccs_distribution_type_e ccs_distribution_type_t;

/**
 * CCS scale types.
 */
enum ccs_scale_type_e {
	/** A linear scale */
	CCS_LINEAR,
	/** A logarithmic scale */
	CCS_LOGARITHMIC,
	/** Guard */
	CCS_SCALE_TYPE_MAX,
	/** Try forcing 32 bits value for bindings */
	CCS_SCALE_TYPE_FORCE_32BIT = INT32_MAX
};

/**
 * A commodity type to represent CCS scale type.
 */
typedef enum ccs_scale_type_e ccs_scale_type_t;

/**
 * Create a new normal distribution of the specified data type. Normal
 * distributions are unidimensional.
 * @param[in] data_type can be either #CCS_NUM_INTEGER or #CCS_NUM_FLOAT
 * @param[in] mu mean of the distribution
 * @param[in] sigma standard deviation of the distribution
 * @param[in] scale an be either #CCS_LINEAR or #CCS_LOGARITHMIC
 * @param[in] quantization quantization of the results, 0 means no quantization.
 *                         Must be a ccs_int_t if \p data_type is
 *                         #CCS_NUM_INTEGER or a ccs_float_t if \p data_type is
 *                         #CCS_NUM_FLOAT
 * @param[out] distribution_ret a pointer to the variable that will contain the
 *                              newly created distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p distribution_ret is NULL; or if \p
 *                             quantization is less than 0
 * @return #CCS_INVALID_TYPE if \p data_type is neither #CCS_NUM_INTEGER nor
 *                            #CCS_NUM_FLOAT
 * @return #CCS_INVALID_SCALE if \p scale is neither #CCS_LINEAR nor
 *                             #CCS_LOGARITHMIC
 * @return #CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new distribution
 */
extern ccs_error_t
ccs_create_normal_distribution(
	ccs_numeric_type_t  data_type,
	ccs_float_t         mu,
	ccs_float_t         sigma,
	ccs_scale_type_t    scale,
	ccs_numeric_t       quantization,
	ccs_distribution_t *distribution_ret);

/**
 * Create a new normal distribution of integer values. Normal
 * distributions are unidimensional.
 * @param[in] mu mean of the distribution
 * @param[in] sigma standard deviation of the distribution
 * @param[in] scale an be either #CCS_LINEAR or #CCS_LOGARITHMIC
 * @param[in] quantization quantization of the results, 0 means no quantization.
 * @param[out] distribution_ret a pointer to the variable that will contain the
 *                              newly created distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p distribution_ret is NULL; or if \p
 *                             quantization is less than 0
 * @return #CCS_INVALID_SCALE if \p scale is neither #CCS_LINEAR nor
 *                             #CCS_LOGARITHMIC
 * @return #CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new distribution
 */
extern ccs_error_t
ccs_create_normal_int_distribution(
	ccs_float_t         mu,
	ccs_float_t         sigma,
	ccs_scale_type_t    scale,
	ccs_int_t           quantization,
	ccs_distribution_t *distribution_ret);

/**
 * Create a new normal distribution of floating point values. Normal
 * distributions are unidimensional.
 * @param[in] mu mean of the distribution
 * @param[in] sigma standard deviation of the distribution
 * @param[in] scale an be either #CCS_LINEAR or #CCS_LOGARITHMIC
 * @param[in] quantization quantization of the results, 0 means no quantization.
 * @param[out] distribution_ret a pointer to the variable that will contain the
 *                              newly created distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p distribution_ret is NULL; or if \p
 *                             quantization is less than 0
 * @return #CCS_INVALID_SCALE if \p scale is neither #CCS_LINEAR nor
 *                             #CCS_LOGARITHMIC
 * @return #CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new distribution
 */
extern ccs_error_t
ccs_create_normal_float_distribution(
	ccs_float_t         mu,
	ccs_float_t         sigma,
	ccs_scale_type_t    scale,
	ccs_float_t         quantization,
	ccs_distribution_t *distribution_ret);

/**
 * Create a new uniform distribution of the specified data type. Uniform
 * distributions are unidimensional.
 * @param[in] data_type can be either #CCS_NUM_INTEGER or #CCS_NUM_FLOAT
 * @param[in] lower the lower bound of the distribution, included. Must be a
 *                  ccs_int_t if \p data_type is #CCS_NUM_INTEGER or a
 *                  ccs_float_t if \p data_type is #CCS_NUM_FLOAT
 * @param[in] upper the upper bound of the distribution, excluded. Must be a
 *                  ccs_int_t if \p data_type is #CCS_NUM_INTEGER or a
 *                  ccs_float_t if \p data_type is #CCS_NUM_FLOAT
 * @param[in] scale an be either #CCS_LINEAR or #CCS_LOGARITHMIC
 * @param[in] quantization quantization of the results, 0 means no quantization.
 *                         Must be a ccs_int_t if \p data_type is
 *                         #CCS_NUM_INTEGER or a ccs_float_t if \p data_type is
 *                         #CCS_NUM_FLOAT
 * @param[out] distribution_ret a pointer to the variable that will contain the
 *                              newly created distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p distribution_ret is NULL; or if \p
 *                             quantization is less than 0; or if the range
 *                             defined by \p lower and \p upper is empty or
 *                             smaller than the quantization; or if \p scale is
 *                             #CCS_LOGARITHMIC \p lower is less or equal to 0
 * @return #CCS_INVALID_TYPE if \p data_type is neither #CCS_NUM_INTEGER nor
 *                            #CCS_NUM_FLOAT
 * @return #CCS_INVALID_SCALE if \p scale is neither #CCS_LINEAR nor
 *                             #CCS_LOGARITHMIC
 * @return #CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new distribution
 */
extern ccs_error_t
ccs_create_uniform_distribution(
	ccs_numeric_type_t  data_type,
	ccs_numeric_t       lower,
	ccs_numeric_t       upper,
	ccs_scale_type_t    scale,
	ccs_numeric_t       quantization,
	ccs_distribution_t *distribution_ret);

/**
 * Create a new uniform distribution of integer values. Uniform
 * distributions are unidimensional.
 * @param[in] lower the lower bound of the distribution, included.
 * @param[in] upper the upper bound of the distribution, excluded.
 * @param[in] scale an be either #CCS_LINEAR or #CCS_LOGARITHMIC
 * @param[in] quantization quantization of the results, 0 means no quantization.
 * @param[out] distribution_ret a pointer to the variable that will contain the
 *                              newly created distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p distribution_ret is NULL; or if \p
 *                             quantization is less than 0; or if the range
 *                             defined by \p lower and \p upper is empty or
 *                             smaller than the quantization; or if \p scale is
 *                             #CCS_LOGARITHMIC \p lower is less or equal to 0
 * @return #CCS_INVALID_SCALE if \p scale is neither #CCS_LINEAR nor
 *                             #CCS_LOGARITHMIC
 * @return #CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new distribution
 */
extern ccs_error_t
ccs_create_uniform_int_distribution(
	ccs_int_t           lower,
	ccs_int_t           upper,
	ccs_scale_type_t    scale,
	ccs_int_t           quantization,
	ccs_distribution_t *distribution_ret);

/**
 * Create a new uniform distribution of floating point values. Uniform
 * distributions are unidimensional.
 * @param[in] lower the lower bound of the distribution, included.
 * @param[in] upper the upper bound of the distribution, excluded.
 * @param[in] scale an be either #CCS_LINEAR or #CCS_LOGARITHMIC
 * @param[in] quantization quantization of the results, 0 means no quantization.
 * @param[out] distribution_ret a pointer to the variable that will contain the
 *                              newly created distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p distribution_ret is NULL; or if \p
 *                             quantization is less than 0; or if the range
 *                             defined by \p lower and \p upper is empty or
 *                             smaller than the quantization; or if \p scale is
 *                             #CCS_LOGARITHMIC \p lower is less or equal to 0
 * @return #CCS_INVALID_SCALE if \p scale is neither #CCS_LINEAR nor
 *                             #CCS_LOGARITHMIC
 * @return #CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new distribution
 */
extern ccs_error_t
ccs_create_uniform_float_distribution(
	ccs_float_t         lower,
	ccs_float_t         upper,
	ccs_scale_type_t    scale,
	ccs_float_t         quantization,
	ccs_distribution_t *distribution_ret);

/**
 * Create a new area proportional distribution of integer values. Roulette
 * distributions are unidimensional.
 * @param[in] num_areas the number of areas
 * @param[in] areas an array of \p num_areas positive floating point values
 *                  representing the probability of a given area to be sampled.
 * @param[out] distribution_ret a pointer to the variable that will contain the
 *                              newly created distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p distribution_ret is NULL; or if \p areas is
 *                             NULL; or if the sum of the areas is 0; or if the
 *                             areas could not be normalized
 * @return #CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new distribution
 */
extern ccs_error_t
ccs_create_roulette_distribution(
	size_t              num_areas,
	ccs_float_t        *areas,
	ccs_distribution_t *distribution_ret);

/**
 * Create a new mixture distribution. Mixture distributions have the same
 * dimensionality as the distributions they are mixing.
 * @param[in] num_distributions the number of distribution to mix together
 * @param[in] distributions an array of \p num_distributions distributions to
 *                          mix together
 * @param[in] weights an array of \p num_distributions positive floating point
 *                    values representing the probability of sampling from one
 *                    of the given distributions
 * @param[out] distribution_ret a pointer to the variable that will contain the
 *                              newly created distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p distribution_ret is NULL; or if
 *                            num_distributions is 0; or if distributions is
 *                            NULL; or if \p weights is NULL; or if the sum of
 *                            the weight is 0; or if the weights could not be
 *                            normalized
 * @return #CCS_INVALID_DISTRIBUTION if the distributions have different
 *                                   dimensions; or if distributions do not all
 *                                   have the same data types
 * @return #CCS_INVALID_OBJECT if at least one of the distributions is not a
 *                             valid CCS distributions
 * @return #CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                            new distribution
 */
extern ccs_error_t
ccs_create_mixture_distribution(
	size_t              num_distributions,
	ccs_distribution_t *distributions,
	ccs_float_t        *weights,
	ccs_distribution_t *distribution_ret);

/**
 * Create a new multivariate distribution. Multivariate distributions have a
 * dimensionality equal to the sum of the dimensionality of their component
 * distributions.
 * @param[in] num_distributions the number of distributions to compose
 * @param[in] distributions an array of \p num_distributions distributions to
 *            compose
 * @param[out] distribution_ret a pointer to the variable that will contain the
 *                              newly created distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p distribution_ret is NULL; or if
 *                             num_distributions is 0; or if distributions is
 *                             NULL
 * @return #CCS_INVALID_OBJECT if at least one of the distributions is not a
 *                              valid CCS distributions
 * @return #CCS_OUT_OF_MEMORY if there was not enough memory to allocate the
 *                             new distribution
 */
extern ccs_error_t
ccs_create_multivariate_distribution(
	size_t              num_distributions,
	ccs_distribution_t *distributions,
	ccs_distribution_t *distribution_ret);

/**
 * Get the type of a distribution.
 * @param[in] distribution
 * @param[out] type_ret a pointer to the variable that will contain the type of
 *                      the distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p type_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 */
extern ccs_error_t
ccs_distribution_get_type(
	ccs_distribution_t       distribution,
	ccs_distribution_type_t *type_ret);

/**
 * Get the dimensionality of a distribution.
 * @param[in] distribution
 * @param[out] dimension_ret a pointer to the variable that will contain the
 *                           dimension  of the distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p dimension_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 */
extern ccs_error_t
ccs_distribution_get_dimension(
	ccs_distribution_t distribution,
	size_t            *dimension_ret);

/**
 * Get the data types of a distribution.
 * @param[in] distribution
 * @param[out] data_types_ret an array of numeric types of the same dimension as
 *                            the distribution, that will contain the data types
 *                            of each dimension of the distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p data_types_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 */
extern ccs_error_t
ccs_distribution_get_data_types(
	ccs_distribution_t  distribution,
	ccs_numeric_type_t *data_types_ret);

/**
 * Get the bounds of a distribution. Bounds are intervals that should contain
 * all possible values returned by a distribution's dimension.
 * @param[in] distribution
 * @param[out] interval_ret an array of intervals of the same dimension as the
 *                          distribution, that will contain the possible
 *                          intervals of each dimension of the distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p interval_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 */
extern ccs_error_t
ccs_distribution_get_bounds(
	ccs_distribution_t distribution,
	ccs_interval_t    *interval_ret);

/**
 * Check if a distribution could return values outside of the given intervals.
 * @param[in] distribution
 * @param[in] intervals an array of intervals of the same dimension as the
 *                      distribution
 * @param[out] oversamplings_ret an array of boolean values of the same
 *                               dimension as the distribution, that contain the
 *                               result of the oversampling test for each
 *                               dimension
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p intervals is NULL; or if \p
 *                             oversamplings_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 */
extern ccs_error_t
ccs_distribution_check_oversampling(
	ccs_distribution_t distribution,
	ccs_interval_t    *intervals,
	ccs_bool_t        *oversamplings_ret);

/**
 * Get the properties of a normal distribution.
 * @param[in] distribution
 * @param[out] mu_ret an optional pointer to the variable that will contain the
 *                    mean of the distribution
 * @param[out] sigma_ret an optional pointer to the variable that will contain
 *                       the standard deviation of the distribution
 * @param[out] scale_ret an optional pointer to the variable that will contain
 *                       the scale used by the distribution
 * @param[out] quantization_ret an optional pointer to the variable that will
 *                              contain the quantization used by the
 *                              distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p mu_ret is NULL and \p sigma_ret is NULL and
 *                             \p scale_ret is NULL and \p quantization_ret is
 *                             NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 * @return #CCS_INVALID_DISTRIBUTION if \p distribution is not a normal
 *                                    distribution
 */
extern ccs_error_t
ccs_normal_distribution_get_properties(
	ccs_distribution_t distribution,
	ccs_float_t       *mu_ret,
	ccs_float_t       *sigma_ret,
	ccs_scale_type_t  *scale_ret,
	ccs_numeric_t     *quantization_ret);

/**
 * Get the properties of a uniform distribution.
 * @param[in] distribution
 * @param[out] lower_ret an optional pointer to the variable that will contain
 *                       the lower bound of the distribution
 * @param[out] upper_ret an optional pointer to the variable that will contain
 *                       the upper bound of the distribution
 * @param[out] scale_ret an optional pointer to the variable that will contain
 *                       the scale used by the distribution
 * @param[out] quantization_ret an optional pointer to the variable that will
 *                              contain the quantization used by the
 *                              distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p lower_ret is NULL and \p upper_ret is NULL
 *                             and \p scale_ret is NULL and \p quantization_ret
 *                             is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 * @return #CCS_INVALID_DISTRIBUTION if \p distribution is not a uniform
 *                                    distribution
 */
extern ccs_error_t
ccs_uniform_distribution_get_properties(
	ccs_distribution_t distribution,
	ccs_numeric_t     *lower_ret,
	ccs_numeric_t     *upper_ret,
	ccs_scale_type_t  *scale_ret,
	ccs_numeric_t     *quantization_ret);

/**
 * Get the number areas of a roulette distribution.
 * @param[in] distribution
 * @param[out] num_areas_ret a pointer to the variable that will contain the
 *                           number of areas of the distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p num_areas_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 * @return #CCS_INVALID_DISTRIBUTION if \p distribution is not a roulette
 *                                    distribution
 */
extern ccs_error_t
ccs_roulette_distribution_get_num_areas(
	ccs_distribution_t distribution,
	size_t            *num_areas_ret);

/**
 * Get the normalized areas of a roulette distribution.
 * @param[in] distribution
 * @param[in] num_areas the number of area that can be contained in \p areas. If
 *                      \p areas is not NULL, \p num_areas must be greater than
 *                      0
 * @param[out] areas an array of \p num_areas floating point values that will
 *                   contain the areas of the distributions, or NULL. If the
 *                   array is too big, extra values are set to 0
 * @param[out] num_areas_ret a pointer to a variable that will contain the
 *                           number of areas that are or would be returned. Can
 *                           be NULL
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p areas is NULL and num_areas is greater than
 *                            0; or if \p areas is NULL and num_areas_ret is
 *                            NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                             distribution
 * @return #CCS_INVALID_DISTRIBUTION if \p distribution is not a roulette
 *                                   distribution
 */
extern ccs_error_t
ccs_roulette_distribution_get_areas(
	ccs_distribution_t distribution,
	size_t             num_areas,
	ccs_float_t       *areas,
	size_t            *num_areas_ret);

/**
 * Set the areas of a roulette distribution.
 * @param[in,out] distribution
 * @param[in] num_areas the number of areas
 * @param[in] areas an array of \p num_areas positive floating point values
 *                  representing the probability of a given area to be sampled.
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p areas is NULL; or if \p num_areas is different
 *                            from the number of areas of the roulette
 *                            distribution; or if the sum of the areas is 0; or
 *                            if the areas could not be normalized
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS distribution
 * @return #CCS_INVALID_DISTRIBUTION if \p distribution is not a roulette
 *                                   distribution
 */
extern ccs_error_t
ccs_roulette_distribution_set_areas(
	ccs_distribution_t distribution,
	size_t             num_areas,
	ccs_float_t       *areas);

/**
 * Get the number of distributions contained in a mixture distribution.
 * @param[in] distribution
 * @param[in] num_distributions_ret a pointer to a variable that will contain
 *                                  the number of distributions contained in the
 *                                  distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p num_distributions_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 * @return #CCS_INVALID_DISTRIBUTION if \p distribution is not a mixture
 *                                    distribution
 */
extern ccs_error_t
ccs_mixture_distribution_get_num_distributions(
	ccs_distribution_t distribution,
	size_t            *num_distributions_ret);

/**
 * Get the distributions contained in a mixture distribution.
 * @param[in] distribution
 * @param[in] num_distributions the number of distributions that can be
 *                              contained in \p distributions. If \p
 *                              distributions is not NULL, \p num_distributions
 *                              must be greater than 0
 * @param[out] distributions an array of \p distributions that will contain the
 *                           returned distributions, or NULL. If the array is
 *                           too big, extra values are set to NULL
 * @param[out] num_distributions_ret a pointer to a variable that will contain
 *                                   the number of distributions that are or
 *                                   would be returned. Can be NULL
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p distributions is NULL and num_distributions
 *                             is greater than 0; or if \p distributions is NULL
 *                             and num_distributions_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 * @return #CCS_INVALID_DISTRIBUTION if \p distribution is not a mixture
 *                                    distribution
 */
extern ccs_error_t
ccs_mixture_distribution_get_distributions(
	ccs_distribution_t  distribution,
	size_t              num_distributions,
	ccs_distribution_t *distributions,
	size_t             *num_distributions_ret);

/**
 * Get the normalized weights of a mixture distribution.
 * @param[in] distribution
 * @param[in] num_weights the number of weight that can be contained in \p
 *                        weights. If \p weights is not NULL, \p num_weights
 *                        must be greater than 0
 * @param[out] weights an array of \p num_weights floating point values that
 *                     will contain the weights of the distributions, or NULL.
 *                     If the array is too big, extra values are set to 0
 * @param[out] num_weights_ret a pointer to a variable that will contain the
 *                             number of weights that are or would be returned.
 *                             Can be NULL
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p weights is NULL and num_weights is greater
 *                             than 0; or if \p weights is NULL and
 *                             num_weights_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 * @return #CCS_INVALID_DISTRIBUTION if \p distribution is not a mixture
 *                                    distribution
 */
extern ccs_error_t
ccs_mixture_distribution_get_weights(
	ccs_distribution_t distribution,
	size_t             num_weights,
	ccs_float_t       *weights,
	size_t            *num_weights_ret);

/**
 * Get the number of distributions contained in a multivariate distribution.
 * @param[in] distribution
 * @param[in] num_distributions_ret a pointer to a variable that will contain
 *                                  the number of distributions contained in the
 *                                  distribution
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p num_distributions_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 * @return #CCS_INVALID_DISTRIBUTION if \p distribution is not a multivariate
 *                                    distribution
 */
extern ccs_error_t
ccs_multivariate_distribution_get_num_distributions(
	ccs_distribution_t distribution,
	size_t            *num_distributions_ret);

/**
 * Get the distributions contained in a multivariate distribution.
 * @param[in] distribution
 * @param[in] num_distributions the number of distributions that can be
 *                              contained in \p distributions. If \p
 *                              distributions is not NULL, \p num_distributions
 *                              must be greater than 0
 * @param[out] distributions an array of \p distributions that will contain the
 *                           returned distributions, or NULL. If the array is
 *                           too big, extra values are set to NULL
 * @param[out] num_distributions_ret a pointer to a variable that will contain
 *                                   the number of distributions that are or
 *                                   would be returned. Can be NULL
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p distributions is NULL and num_distributions
 *                             is greater than 0; or if \p distributions is NULL
 *                             and num_distributions_ret is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution
 * @return #CCS_INVALID_DISTRIBUTION if \p distribution is not a multivariate
 *                                    distribution
 */
extern ccs_error_t
ccs_multivariate_distribution_get_distributions(
	ccs_distribution_t  distribution,
	size_t              num_distributions,
	ccs_distribution_t *distributions,
	size_t             *num_distributions_ret);

/**
 * Get a random sample from a distribution.
 * @param[in] distribution
 * @param[in,out] rng the random number generator to use
 * @param[out] values an array of numeric values the same dimension as the
 *             distribution. Will contain the sampled values. The type of the
 *             numeric returned depends on the data type of each dimension (see
 *             #ccs_distribution_get_data_types)
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p values is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution; or if \p rng is not a valid CCS
 *                              rng
 */
extern ccs_error_t
ccs_distribution_sample(
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	ccs_numeric_t     *values);

/**
 * Get a collection of random samples from a distribution.
 * @param[in] distribution
 * @param[in,out] rng the random number generator to use
 * @param[in] num_samples the number of samples to get
 * @param[out] values an array of numeric values. The dimension of the array
 *                    should be the dimension of the distribution times \p
 *                    num_samples. Values will be in an array of structures
 *                    ordering, so values from a single sample will be
 *                    contiguous in memory
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p values is NULL and \p num_samples is
 *                             greater than 0
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution; or if \p rng is not a valid CCS
 *                              rng
 */
extern ccs_error_t
ccs_distribution_samples(
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	size_t             num_samples,
	ccs_numeric_t     *values);

/**
 * Get a collection of random samples from a distribution. Each sample is stride
 * ccs_numeric_t element apart in memory. A stride equal to the the distribution
 * dimension is equivalent to calling #ccs_distribution_samples.
 * @param[in] distribution
 * @param[in,out] rng the random number generator to use
 * @param[in] num_samples the number of samples to get
 * @param[in] stride the distance in memory that will separate two samples
 *                   successive samples. The value is given in number of
 *                   ccs_numeric_t
 * @param[out] values an array of numeric values. The dimension of the array
 *                    should be the stride times \p num_samples minus stride
 *                    plus the dimension of the distribution. Values will be in
 *                    an array of structures ordering, so values from a single
 *                    sample will be contiguous in memory
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p values is NULL and \p num_samples is
 *                             greater than 0; or if \p stride is less than the
 *                             distribution dimension
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution; or if \p rng is not a valid CCS
 *                              rng
 */
extern ccs_error_t
ccs_distribution_strided_samples(
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	size_t             num_samples,
	size_t             stride,
	ccs_numeric_t     *values);

/**
 * Get a collection of random samples from a distribution. Each sample
 * components will be stored contiguously in it's own array.
 * @param[in] distribution
 * @param[in,out] rng the random number generator to use
 * @param[in] num_samples the number of samples to get
 * @param[out] values an array of arrays of numeric values. The array dimension
 *                    is the dimension of the distribution, while the contained
 *                    arrays have a dimension of num_samples. If a value inside
 *                    the array is NULL this dimension is ignored.
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p values is NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution; or if \p rng is not a valid CCS
 *                              rng
 */
extern ccs_error_t
ccs_distribution_soa_samples(
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	size_t             num_samples,
	ccs_numeric_t    **values);
/**
 * Get a collection of random parameters' samples by sampling a
 * distribution.
 * @param[in] distribution
 * @param[in,out] rng the random number generator to use
 * @param[in] parameters an array of parameters. The dimension of the
 *                            array must be qual to the dimension of the
 *                            distribution
 * @param[in] num_samples the number of samples to get
 * @param[out] values an array of datum values. The dimension of the array
 *                    should be the dimension of the distribution times \p
 *                    num_samples. Values will be in an array of structures
 *                    ordering, so values from a single sample will be
 *                    contiguous in memory
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p values is NULL and \p num_samples is
 *                             greater than 0; or if \p parameters is NULL
 *                             and \p num_samples is greater than 0
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution; or if \p rng is not a valid CCS
 *                              rng; or if at least one of the parameters
 *                              provided is NULL
 */
extern ccs_error_t
ccs_distribution_parameters_samples(
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	ccs_parameter_t   *parameters,
	size_t             num_samples,
	ccs_datum_t       *values);

/**
 * Get a random parameters' sample by sampling a distribution.
 * @param[in] distribution
 * @param[in,out] rng the random number generator to use
 * @param[in] parameters an array of parameters. The dimension of the
 *                            array must be qual to the dimension of the
 *                            distribution
 * @param[out] values an array of datum of the same dimension as the
 *             distribution. Will contain the sampled values.
 * @return #CCS_SUCCESS on success
 * @return #CCS_INVALID_VALUE if \p values is NULL; or if \p parameters is
 *                             NULL
 * @return #CCS_INVALID_OBJECT if \p distribution is not a valid CCS
 *                              distribution; or if \p rng is not a valid CCS
 *                              rng; or if at least one of the parameters
 *                              provided is NULL
 */
extern ccs_error_t
ccs_distribution_parameters_sample(
	ccs_distribution_t distribution,
	ccs_rng_t          rng,
	ccs_parameter_t   *parameters,
	ccs_datum_t       *values);

#ifdef __cplusplus
}
#endif

#endif //_CCS_DISTRIBUTION_H
