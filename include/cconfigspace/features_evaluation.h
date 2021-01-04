#ifndef _CCS_FEATURES_EVALUATION_H
#define _CCS_FEATURES_EVALUATION_H

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_result_t
ccs_create_features_evaluation(ccs_objective_space_t      objective_space,
                               ccs_configuration_t        configuration,
                               ccs_features_t             features,
                               ccs_result_t               error,
                               size_t                     num_values,
                               ccs_datum_t               *values,
                               void                      *user_data,
                               ccs_features_evaluation_t *features_evaluation_ret);

extern ccs_result_t
ccs_features_evaluation_get_objective_space(
		ccs_features_evaluation_t  features_evaluation,
		ccs_objective_space_t     *objective_space_ret);

extern ccs_result_t
ccs_features_evaluation_get_configuration(
		ccs_features_evaluation_t  features_evaluation,
		ccs_configuration_t       *configuration_ret);

extern ccs_result_t
ccs_features_evaluation_get_features(
		ccs_features_evaluation_t  features_evaluation,
		ccs_features_t            *features_ret);

extern ccs_result_t
ccs_features_evaluation_get_user_data(ccs_features_evaluation_t   features_evaluation,
                                      void                      **user_data_ret);

extern ccs_result_t
ccs_features_evaluation_get_error(ccs_features_evaluation_t  features_evaluation,
                                  ccs_result_t              *error_ret);

extern ccs_result_t
ccs_features_evaluation_set_error(ccs_features_evaluation_t features_evaluation,
                                  ccs_result_t              error);

extern ccs_result_t
ccs_features_evaluation_get_value(ccs_features_evaluation_t  features_evaluation,
                                  size_t                     index,
                                  ccs_datum_t               *value_ret);

extern ccs_result_t
ccs_features_evaluation_set_value(ccs_features_evaluation_t features_evaluation,
                                  size_t                    index,
                                  ccs_datum_t               value);

extern ccs_result_t
ccs_features_evaluation_get_values(ccs_features_evaluation_t  features_evaluation,
                                   size_t                     num_values,
                                   ccs_datum_t               *values,
                                   size_t                    *num_values_ret);

extern ccs_result_t
ccs_features_evaluation_get_value_by_name(ccs_features_evaluation_t  features_evaluation,
                                          const char                *name,
                                          ccs_datum_t               *value_ret);

extern ccs_result_t
ccs_features_evaluation_get_objective_value(
		ccs_features_evaluation_t  features_evaluation,
		size_t                     index,
		ccs_datum_t               *value_ret);

extern ccs_result_t
ccs_features_evaluation_get_objective_values(
		ccs_features_evaluation_t  features_evaluation,
		size_t                     num_values,
		ccs_datum_t               *values,
		size_t                    *num_values_ret);

extern ccs_result_t
ccs_features_evaluation_cmp(ccs_features_evaluation_t  features_evaluation,
                            ccs_features_evaluation_t  other_features_evaluation,
                            ccs_comparison_t          *result_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_FEATURES_EVALUATION_H
