#ifndef _CCS_EVALUATION_H
#define _CCS_EVALUATION_H

#ifdef __cplusplus
extern "C" {
#endif

enum ccs_comparison_e {
	CCS_BETTER = -1,
	CCS_EQUIVALENT = 0,
	CCS_WORSE = 1,
	CCS_NOT_COMPARABLE = 2,
	CCS_COMPARISON_MAX,
	CCS_COMPARISON_FORCE_32BIT = INT_MAX
};
typedef enum ccs_comparison_e ccs_comparison_t;

extern ccs_error_t
ccs_create_evaluation(ccs_objective_space_t  objective_space,
                      ccs_configuration_t    configuration,
                      ccs_error_t            error,
                      size_t                 num_values,
                      ccs_datum_t           *values,
                      void                  *user_data,
                      ccs_evaluation_t      *evaluation_ret);

extern ccs_error_t
ccs_evaluation_get_objective_space(ccs_evaluation_t       evaluation,
                                   ccs_objective_space_t *objective_space_ret);

extern ccs_error_t
ccs_evaluation_get_configuration(ccs_evaluation_t     evaluation,
                                 ccs_configuration_t *configuration_ret);

extern ccs_error_t
ccs_evaluation_get_user_data(ccs_evaluation_t   evaluation,
                             void             **user_data_ret);

extern ccs_error_t
ccs_evaluation_get_error(ccs_evaluation_t  evaluation,
                         ccs_error_t      *error_ret);

extern ccs_error_t
ccs_evaluation_set_error(ccs_evaluation_t evaluation,
                         ccs_error_t      error);

extern ccs_error_t
ccs_evaluation_get_value(ccs_evaluation_t  evaluation,
                         size_t            index,
                         ccs_datum_t      *value_ret);

extern ccs_error_t
ccs_evaluation_set_value(ccs_evaluation_t evaluation,
                         size_t           index,
                         ccs_datum_t      value);

extern ccs_error_t
ccs_evaluation_get_values(ccs_evaluation_t  evaluation,
                          size_t            num_values,
                          ccs_datum_t      *values,
                          size_t           *num_values_ret);

extern ccs_error_t
ccs_evaluation_get_value_by_name(ccs_evaluation_t  evaluation,
                                 const char       *name,
                                 ccs_datum_t      *value_ret);

extern ccs_error_t
ccs_evaluation_get_objective_value(ccs_evaluation_t  evaluation,
                                   size_t            index,
                                   ccs_datum_t      *value_ret);

extern ccs_error_t
ccs_evaluation_get_objective_values(ccs_evaluation_t  evaluation,
                                    size_t            num_values,
                                    ccs_datum_t      *values,
                                    size_t           *num_values_ret);

extern ccs_error_t
ccs_evaluation_cmp(ccs_evaluation_t  evaluation,
                   ccs_evaluation_t  other_evaluation,
                   ccs_comparison_t  *result_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_EVALUATION_H
