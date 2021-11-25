#ifndef _CCS_DISTRIBUTION
#define _CCS_DISTRIBUTION

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

// Distribution
extern ccs_result_t
ccs_create_normal_distribution(ccs_numeric_type_t  data_type,
                               ccs_float_t         mu,
                               ccs_float_t         sigma,
                               ccs_scale_type_t    scale,
                               ccs_numeric_t       quantization,
                               ccs_distribution_t *distribution_ret);

extern ccs_result_t
ccs_create_normal_int_distribution(ccs_float_t         mu,
                                   ccs_float_t         sigma,
                                   ccs_scale_type_t    scale,
                                   ccs_int_t           quantization,
                                   ccs_distribution_t *distribution_ret);

extern ccs_result_t
ccs_create_normal_float_distribution(ccs_float_t         mu,
                                     ccs_float_t         sigma,
                                     ccs_scale_type_t    scale,
                                     ccs_float_t         quantization,
                                     ccs_distribution_t *distribution_ret);

extern ccs_result_t
ccs_create_uniform_distribution(ccs_numeric_type_t  data_type,
                                ccs_numeric_t       lower,
                                ccs_numeric_t       upper,
                                ccs_scale_type_t    scale,
                                ccs_numeric_t       quantization,
                                ccs_distribution_t *distribution_ret);

extern ccs_result_t
ccs_create_uniform_int_distribution(ccs_int_t           lower,
                                    ccs_int_t           upper,
                                    ccs_scale_type_t    scale,
                                    ccs_int_t           quantization,
                                    ccs_distribution_t *distribution_ret);

extern ccs_result_t
ccs_create_uniform_float_distribution(ccs_float_t         lower,
                                      ccs_float_t         upper,
                                      ccs_scale_type_t    scale,
                                      ccs_float_t         quantization,
                                      ccs_distribution_t *distribution_ret);

extern ccs_result_t
ccs_create_roulette_distribution(size_t              num_areas,
                                 ccs_float_t        *areas,
                                 ccs_distribution_t *distribution_ret);

extern ccs_result_t
ccs_create_mixture_distribution(size_t              num_distributions,
                                ccs_distribution_t *distributions,
                                ccs_float_t        *weights,
                                ccs_distribution_t *distribution_ret);

extern ccs_result_t
ccs_create_multivariate_distribution(size_t              num_distributions,
                                     ccs_distribution_t *distributions,
                                     ccs_distribution_t *distribution_ret);

//   Accessors
extern ccs_result_t
ccs_distribution_get_type(ccs_distribution_t       distribution,
                          ccs_distribution_type_t *type_ret);

extern ccs_result_t
ccs_distribution_get_dimension(ccs_distribution_t  distribution,
                               size_t             *dimension);

extern ccs_result_t
ccs_distribution_get_data_types(ccs_distribution_t       distribution,
                                ccs_numeric_type_t      *data_types_ret);

extern ccs_result_t
ccs_distribution_get_bounds(ccs_distribution_t  distribution,
                            ccs_interval_t     *interval_ret);

extern ccs_result_t
ccs_distribution_check_oversampling(ccs_distribution_t  distribution,
                                    ccs_interval_t     *intervals,
                                    ccs_bool_t         *oversamplings);

extern ccs_result_t
ccs_normal_distribution_get_parameters(ccs_distribution_t  distribution,
                                       ccs_float_t        *mu_ret,
                                       ccs_float_t        *sigma_ret,
                                       ccs_scale_type_t   *scale_ret,
                                       ccs_numeric_t      *quantization_ret);

extern ccs_result_t
ccs_uniform_distribution_get_parameters(ccs_distribution_t  distribution,
                                        ccs_numeric_t      *lower_ret,
                                        ccs_numeric_t      *upper_ret,
                                        ccs_scale_type_t   *scale_ret,
                                        ccs_numeric_t      *quantization_ret);

extern ccs_result_t
ccs_roulette_distribution_get_num_areas(ccs_distribution_t  distribution,
                                        size_t             *num_areas_ret);

extern ccs_result_t
ccs_roulette_distribution_get_areas(ccs_distribution_t  distribution,
                                    size_t              num_areas,
                                    ccs_float_t        *areas,
                                    size_t             *num_areas_ret);

extern ccs_result_t
ccs_mixture_distribution_get_num_distributions(ccs_distribution_t  distribution,
                                               size_t             *num_distributions_ret);

extern ccs_result_t
ccs_mixture_distribution_get_distributions(ccs_distribution_t  distribution,
                                           size_t              num_distributions,
                                           ccs_distribution_t *distributions,
                                           size_t             *num_distributions_ret);

extern ccs_result_t
ccs_mixture_distribution_get_weights(ccs_distribution_t  distribution,
                                     size_t              num_weights,
                                     ccs_float_t        *weights,
                                     size_t             *num_weights_ret);

extern ccs_result_t
ccs_multivariate_distribution_get_num_distributions(ccs_distribution_t  distribution,
                                                    size_t             *num_distributions_ret);

extern ccs_result_t
ccs_multivariate_distribution_get_distributions(ccs_distribution_t  distribution,
                                                size_t              num_distributions,
                                                ccs_distribution_t *distributions,
                                                size_t             *num_distributions_ret);

//   Sampling Interface
extern ccs_result_t
ccs_distribution_sample(ccs_distribution_t  distribution,
                        ccs_rng_t           rng,
                        ccs_numeric_t      *value);

extern ccs_result_t
ccs_distribution_samples(ccs_distribution_t  distribution,
                         ccs_rng_t           rng,
                         size_t              num_values,
                         ccs_numeric_t      *values);

// Stride between elements given in number of ccs_numeric_t.
// stride equal to the the distribution dimension is
// equivalent to ccs_distribution_samples
extern ccs_result_t
ccs_distribution_strided_samples(ccs_distribution_t  distribution,
                                 ccs_rng_t           rng,
                                 size_t              num_values,
                                 size_t              stride,
                                 ccs_numeric_t      *values);

extern ccs_result_t
ccs_distribution_soa_samples(ccs_distribution_t   distribution,
                             ccs_rng_t            rng,
                             size_t               num_values,
                             ccs_numeric_t      **values);

extern ccs_result_t
ccs_distribution_hyperparameters_samples(ccs_distribution_t    distribution,
                                         ccs_rng_t             rng,
                                         ccs_hyperparameter_t *hyperparameters,
                                         size_t                num_values,
                                         ccs_datum_t          *values);

extern ccs_result_t
ccs_distribution_hyperparameters_sample(ccs_distribution_t    distribution,
                                        ccs_rng_t             rng,
                                        ccs_hyperparameter_t *hyperparameters,
                                        ccs_datum_t          *values);

#ifdef __cplusplus
}
#endif

#endif //_CCS_DISTRIBUTION
