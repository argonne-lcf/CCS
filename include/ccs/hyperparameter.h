enum ccs_distribution_type_e {
	CCS_UNIFORM,
	CCS_NORMAL,
	CCS_DISTRIBUTION_TYPE_MAX,
	CCS_DISTRIBUTION_FORCE_32BIT = MAX_INT
};

typedef enum ccs_distribution_type_e ccs_distribution_type_t;

enum ccs_scale_type_e {
	CSS_LINEAR,
	CSS_LOGARITHMIC,
	CCS_SCALE_TYPE_MAX,
	CCS_SCALE_TYPE_FORCE_32BIT = MAX_INT
};

typedef enum ccs_scale_type_e ccs_scale_type_t;

typedef enum ccs_hyperparameter_type_e {
	CCS_NUMERICAL,
	CCS_CATEGORICAL.
	CCS_ORDINAL,
	CCS_HYPERPARAMETER_TYPE_MAX,
	CCS_HYPERPARAMETER_TYPE_FORCE_32BIT = MAX_INT
};

typedef enum ccs_hyperparameter_type_e ccs_hyperparameter_type_t;

typedef struct _ccs_rng_s          *ccs_rng_t;
typedef struct _ccs_distribution_s *ccs_distribution_t;

// Distribution
extern ccs_error_t
ccs_create_normal_distribution(ccs_data_type_t     data_type,
                               ccs_data_t          mu,
                               ccs_data_t          sigma,
                               ccs_scale_type_t    scale,
                               ccs_data_t          quantization,
                               ccs_distribution_t *distribution_ret);

extern ccs_error_t
ccs_create_normal_int_distribution(int64_t             mu,
                                   int64_t             sigma,
                                   ccs_scale_type_t    scale,
                                   int64_t             quantization,
                                   ccs_distribution_t *distribution_ret);

extern ccs_error_t
ccs_create_normal_float_distribution(double mu,
                                     double sigma,
                                     ccs_scale_type_t    scale,
                                     double              quantization,
                                     ccs_distribution_t *distribution_ret);

extern ccs_error_t
ccs_create_uniform_int_distribution(int64_t lower,
                                    int64_t upper,
                                    ccs_scale_type_t    scale,
                                    int64_t             quantization,
                                    ccs_distribution_t *distribution_ret); 

extern ccs_error_t
ccs_create_uniform_float_distribution(double lower,
                                      double upper,
                                      ccs_scale_type_t    scale,
                                      double              quantization,
                                      ccs_distribution_t *distribution_ret);

extern ccs_error_t
ccs_retain_distribution(ccs_distribution_t distribution);

extern ccs_error_t
ccs_release_distribution(ccs_distribution_t distribution);

//   Accessors
extern ccs_error_t
ccs_distribution_get_type(ccs_distribution_t       distribution,
                          ccs_distribution_type_t *type_ret);

extern ccs_error_t
ccs_distribution_get_data_type(ccs_distribution_t       distribution,
                               ccs_data_type_t         *data_type_ret);

extern ccs_error_t
ccs_distribution_get_scale_type(ccs_distribution_t  distribution,
                                ccs_scale_type_t   *scale_type_ret);

extern ccs_error_t
ccs_distribution_get_quantization(ccs_distribution_t  distribution,
                                  ccs_data_t         *quantization);

extern ccs_error_t
ccs_normal_distribution_get_parameters(ccs_distribution_t  distribution,
                                       ccs_data_t         *mu_ret,
                                       ccs_data_t         *sigma_ret);

extern ccs_error_t
ccs_uniform_distribution_get_parameters(ccs_distribution_t  distribution,
                                        ccs_data_t         *lower,
                                        ccs_data_t         *upper);

//   Sampling Interface
extern ccs_error_t
ccs_distribution_sample(ccs_distribution_t  distribution,
                        ccs_rng_t           rng,
                        ccs_data_t         *value);

extern ccs_error_t
ccs_distribution_samples(ccs_distribution_t  distribution,
                         ccs_rng_t           rng,
                         size_t              num_values,
                         ccs_data_t         *values);

// Hyperparameter Interface
extern ccs_error_t
ccs_retain_hyperparameter(ccs_hyperparameter_t hyperparameter);

extern ccs_error_t
ccs_release_hyperparameter(ccs_hyperparameter_t hyperparameter);
