#ifndef _CCS_HYPERPARAMETER_H
#define _CCS_HYPERPARAMETER_H

#ifdef __cplusplus
extern "C" {
#endif

enum ccs_hyperparameter_type_e {
	CCS_NUMERICAL,
	CCS_CATEGORICAL,
	CCS_ORDINAL,
	CCS_HYPERPARAMETER_TYPE_MAX,
	CCS_HYPERPARAMETER_TYPE_FORCE_32BIT = INT_MAX
};

typedef enum ccs_hyperparameter_type_e ccs_hyperparameter_type_t;

// Hyperparameter Interface
extern ccs_error_t
_ccs_create_numerical_hyperparameter(const char           *name,
                                     ccs_numeric_type_t    data_type,
                                     ccs_numeric_t         lower,
                                     ccs_numeric_t         upper,
                                     ccs_numeric_t         quantization,
                                     ccs_numeric_t         default_value,
                                     ccs_distribution_t    distribution,
                                     void                 *user_data,
                                     ccs_hyperparameter_t *hyperparameter_ret);
#define ccs_create_numerical_hyperparameter(n, t, l, u, q, d, dis, us, h) \
	_ccs_create_numerical_hyperparameter(n, t, (ccs_value_t)(l), (ccs_value_t)(u), (ccs_value_t)(q), (ccs_value_t)(d), dis, us, h)

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
ccs_hyperparameter_get_type(ccs_hyperparameter_t       hyperparameter,
                            ccs_hyperparameter_type_t *type_ret);

extern ccs_error_t
ccs_hyperparameters_get_default_value(ccs_hyperparameter_t  hyperparameter,
                                      ccs_datum_t          *value_ret);

extern ccs_error_t
ccs_hyperparameters_get_name(ccs_hyperparameter_t   hyperparameter,
                             const char           **name_ret);

extern ccs_error_t
ccs_hyperparameters_get_user_data(ccs_hyperparameter_t   hyperparameter,
                                  void                 **user_data_ret);
extern ccs_error_t
ccs_hyperparameter_get_distribution(ccs_hyperparameter_t  hyperparameter,
                                    ccs_distribution_t   *distribution);
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

#ifdef __cplusplus
}
#endif

#endif //_CCS_HYPERPARAMETER_H
