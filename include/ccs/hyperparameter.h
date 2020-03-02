#ifndef _CCS_HYPERPARAMETER_H
#define _CCS_HYPERPARAMETER_H

enum ccs_distribution_type_e {
	CCS_UNIFORM,
	CCS_NORMAL,
	CCS_DISTRIBUTION_TYPE_MAX,
	CCS_DISTRIBUTION_FORCE_32BIT = INT_MAX
};

typedef enum ccs_distribution_type_e ccs_distribution_type_t;

enum ccs_scale_type_e {
	CSS_LINEAR,
	CSS_LOGARITHMIC,
	CCS_SCALE_TYPE_MAX,
	CCS_SCALE_TYPE_FORCE_32BIT = INT_MAX
};

typedef enum ccs_scale_type_e ccs_scale_type_t;

enum ccs_hyperparameter_type_e {
	CCS_NUMERICAL,
	CCS_CATEGORICAL,
	CCS_ORDINAL,
	CCS_HYPERPARAMETER_TYPE_MAX,
	CCS_HYPERPARAMETER_TYPE_FORCE_32BIT = INT_MAX
};

typedef enum ccs_hyperparameter_type_e ccs_hyperparameter_type_t;

typedef struct _ccs_rng_s             *ccs_rng_t;

// Distribution
extern ccs_error_t
ccs_create_distribution(ccs_distribution_type_t distribution_type,
                        ccs_data_type_t         data_type,
                        ccs_scale_type_t        scale,
                        ccs_value_t             quantization,
                        ccs_value_t            *parameters,
                        ccs_distribution_t     *distribution_ret);

extern ccs_error_t
ccs_create_normal_distribution(ccs_data_type_t     data_type,
                               ccs_value_t         mu,
                               ccs_value_t         sigma,
                               ccs_scale_type_t    scale,
                               ccs_value_t         quantization,
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
                                  ccs_datum_t        *quantization);

extern ccs_error_t
ccs_normal_distribution_get_parameters(ccs_distribution_t  distribution,
                                       ccs_datum_t        *mu_ret,
                                       ccs_datum_t        *sigma_ret);

extern ccs_error_t
ccs_uniform_distribution_get_parameters(ccs_distribution_t  distribution,
                                        ccs_datum_t        *lower,
                                        ccs_datum_t        *upper);

//   Sampling Interface
extern ccs_error_t
ccs_distribution_sample(ccs_distribution_t  distribution,
                        ccs_rng_t           rng,
                        ccs_datum_t        *value);

extern ccs_error_t
ccs_distribution_samples(ccs_distribution_t  distribution,
                         ccs_rng_t           rng,
                         size_t              num_values,
                         ccs_datum_t        *values);

extern ccs_error_t
ccs_distribution_is_value_valid(ccs_distribution_t distribution,
                                ccs_datum_t        value);

// Hyperparameter Interface
extern ccs_error_t
ccs_create_numerical_hyperparameter(const char           *name,
                                    void                 *user_data,
                                    ccs_distribution_t    distribution,
                                    ccs_datum_t           default_value,
                                    ccs_hyperparameter_t *hyperparameter_ret);

extern ccs_error_t
ccs_create_categorical_hyperparameter(const char           *name,
                                      void                 *user_data,
                                      size_t                num_possible_values,
                                      ccs_datum_t          *possible_values,
                                      ccs_datum_t           default_value,
                                      ccs_datum_t          *weights,
                                      ccs_hyperparameter_t *hyperparameter_ret);

extern ccs_error_t
ccs_create_ordinal_hyperparameters(const char           *name,
                                   void                 *user_data,
                                   size_t                num_possible_values,
                                   ccs_datum_t          *possible_values,
                                   ccs_hyperparameter_t *hyperparameter_ret);

//   Accessors

extern ccs_error_t
ccs_hyperparameters_get_default_value(ccs_hyperparameter_t  hyperparameter,
                                      ccs_datum_t          *value);

extern ccs_error_t
ccs_hyperparameters_get_name(ccs_hyperparameter_t   hyperparameter,
                             const char           **name);

extern ccs_error_t
ccs_hyperparameters_get_user_data(ccs_hyperparameter_t   hyperparameter,
                                  void                 **user_data);
//   Sampling Interface
extern ccs_error_t
ccs_hyperparameters_sample(ccs_hyperparameter_t  hyperparameter,
                           ccs_rng_t             rng,
                           ccs_datum_t          *value);

extern ccs_error_t
ccs_hyperparameters_samples(ccs_hyperparameter_t  hyperparameter,
                            ccs_rng_t             rng,
                            size_t                num_values,
                            ccs_datum_t          *values);


#endif //_CCS_HYPERPARAMETER_H
