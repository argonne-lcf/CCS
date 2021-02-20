#ifndef _CCS_FEATURES_SPACE
#define _CCS_FEATURES_SPACE

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_result_t
ccs_create_features_space(const char           *name,
                          void                 *user_data,
                          ccs_features_space_t *features_space_ret);

extern ccs_result_t
ccs_features_space_get_name(ccs_features_space_t   features_space,
                            const char           **name_ret);

extern ccs_result_t
ccs_features_space_get_user_data(ccs_features_space_t   features_space,
                                 void                 **user_data_ret);

extern ccs_result_t
ccs_features_space_add_hyperparameter(ccs_features_space_t features_space,
                                      ccs_hyperparameter_t hyperparameter);

extern ccs_result_t
ccs_features_space_add_hyperparameters(ccs_features_space_t  features_space,
                                       size_t                num_hyperparameters,
                                       ccs_hyperparameter_t *hyperparameters);

extern ccs_result_t
ccs_features_space_get_num_hyperparameters(ccs_features_space_t  features_space,
                                           size_t               *num_hyperparameters_ret);

extern ccs_result_t
ccs_features_space_get_hyperparameter(ccs_features_space_t  features_space,
                                      size_t                index,
                                      ccs_hyperparameter_t *hyperparameter_ret);

extern ccs_result_t
ccs_features_space_get_hyperparameter_by_name(
		ccs_features_space_t  features_space,
		const char *          name,
		ccs_hyperparameter_t *hyperparameter_ret);

extern ccs_result_t
ccs_features_space_get_hyperparameter_index_by_name(
		ccs_features_space_t  features_space,
		const char           *name,
		size_t               *index_ret);

extern ccs_result_t
ccs_features_space_get_hyperparameter_index(
		ccs_features_space_t  features_space,
		ccs_hyperparameter_t  hyperparameter,
		size_t               *index_ret);

extern ccs_result_t
ccs_features_space_get_hyperparameter_indexes(
		ccs_features_space_t  features_space,
		size_t                num_hyperparameters,
		ccs_hyperparameter_t *hyperparameters,
		size_t               *indexes);

extern ccs_result_t
ccs_features_space_get_hyperparameters(ccs_features_space_t  features_space,
                                       size_t                num_hyperparameters,
                                       ccs_hyperparameter_t *hyperparameters,
                                       size_t               *num_hyperparameters_ret);

extern ccs_result_t
ccs_features_space_validate_value(ccs_features_space_t  features_space,
                                  size_t                index,
                                  ccs_datum_t           value,
                                  ccs_datum_t          *value_ret);

extern ccs_result_t
ccs_features_space_check_features(ccs_features_space_t features_space,
                                  ccs_features_t       features);

extern ccs_result_t
ccs_features_space_check_features_values(ccs_features_space_t  features_space,
                                         size_t                num_values,
                                         ccs_datum_t          *values);

#ifdef __cplusplus
}
#endif

#endif //_CCS_FEATURES_SPACE
