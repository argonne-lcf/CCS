#ifndef _CCS_DISTRIBUTION
#define _CCS_DISTRIBUTION

#ifdef __cplusplus
extern "C" {
#endif

enum ccs_distribution_type_e {
	CCS_UNIFORM,
	CCS_NORMAL,
	CCS_ROULETTE,
	CCS_DISTRIBUTION_TYPE_MAX,
	CCS_DISTRIBUTION_TYPE_FORCE_32BIT = INT_MAX
};

typedef enum ccs_distribution_type_e ccs_distribution_type_t;

enum ccs_scale_type_e {
	CCS_LINEAR,
	CCS_LOGARITHMIC,
	CCS_SCALE_TYPE_MAX,
	CCS_SCALE_TYPE_FORCE_32BIT = INT_MAX
};

typedef enum ccs_scale_type_e ccs_scale_type_t;

// Distribution
extern ccs_error_t
ccs_create_normal_distribution(ccs_numeric_type_t  data_type,
                               ccs_float_t         mu,
                               ccs_float_t         sigma,
                               ccs_scale_type_t    scale,
                               ccs_numeric_t       quantization,
                               ccs_distribution_t *distribution_ret);

extern ccs_error_t
ccs_create_normal_int_distribution(ccs_float_t         mu,
                                   ccs_float_t         sigma,
                                   ccs_scale_type_t    scale,
                                   ccs_int_t           quantization,
                                   ccs_distribution_t *distribution_ret);

extern ccs_error_t
ccs_create_normal_float_distribution(ccs_float_t         mu,
                                     ccs_float_t         sigma,
                                     ccs_scale_type_t    scale,
                                     ccs_float_t         quantization,
                                     ccs_distribution_t *distribution_ret);

extern ccs_error_t
ccs_create_uniform_distribution(ccs_numeric_type_t  data_type,
                                ccs_numeric_t       lower,
                                ccs_numeric_t       upper,
                                ccs_scale_type_t    scale_type,
                                ccs_numeric_t       quantization,
                                ccs_distribution_t *distribution_ret);

extern ccs_error_t
ccs_create_uniform_int_distribution(ccs_int_t           lower,
                                    ccs_int_t           upper,
                                    ccs_scale_type_t    scale,
                                    ccs_int_t           quantization,
                                    ccs_distribution_t *distribution_ret);

extern ccs_error_t
ccs_create_uniform_float_distribution(ccs_float_t         lower,
                                      ccs_float_t         upper,
                                      ccs_scale_type_t    scale,
                                      ccs_float_t         quantization,
                                      ccs_distribution_t *distribution_ret);

extern ccs_error_t
ccs_create_roulette_distribution(size_t              num_areas,
                                 ccs_float_t        *areas,
                                 ccs_distribution_t *distribution_ret);

//   Accessors
extern ccs_error_t
ccs_distribution_get_type(ccs_distribution_t       distribution,
                          ccs_distribution_type_t *type_ret);

extern ccs_error_t
ccs_distribution_get_data_type(ccs_distribution_t       distribution,
                               ccs_numeric_type_t      *data_type_ret);

extern ccs_error_t
ccs_distribution_get_dimension(ccs_distribution_t  distribution,
                               size_t             *dimension);

extern ccs_error_t
ccs_distribution_get_scale_type(ccs_distribution_t  distribution,
                                ccs_scale_type_t   *scale_type_ret);

extern ccs_error_t
ccs_distribution_get_quantization(ccs_distribution_t  distribution,
                                  ccs_numeric_t      *quantization);

extern ccs_error_t
ccs_distribution_get_bounds(ccs_distribution_t  distribution,
                            ccs_interval_t     *interval_ret);

extern ccs_error_t
ccs_distribution_check_oversampling(ccs_distribution_t  distribution,
                                    ccs_interval_t     *interval,
                                    ccs_bool_t         *oversampling_ret);

extern ccs_error_t
ccs_normal_distribution_get_parameters(ccs_distribution_t  distribution,
                                       ccs_float_t        *mu_ret,
                                       ccs_float_t        *sigma_ret);

extern ccs_error_t
ccs_uniform_distribution_get_parameters(ccs_distribution_t  distribution,
                                        ccs_numeric_t      *lower_ret,
                                        ccs_numeric_t      *upper_ret);

extern ccs_error_t
ccs_roulette_distribution_get_num_areas(ccs_distribution_t  distribution,
                                        size_t             *num_areas_ret);

extern ccs_error_t
ccs_roulette_distribution_get_areas(ccs_distribution_t  distribution,
                                    size_t              num_areas,
                                    ccs_float_t        *areas);

//   Sampling Interface
extern ccs_error_t
ccs_distribution_sample(ccs_distribution_t  distribution,
                        ccs_rng_t           rng,
                        ccs_numeric_t      *value);

extern ccs_error_t
ccs_distribution_samples(ccs_distribution_t  distribution,
                         ccs_rng_t           rng,
                         size_t              num_values,
                         ccs_numeric_t      *values);

#ifdef __cplusplus
}
#endif

#endif //_CCS_DISTRIBUTION
