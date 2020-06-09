#ifndef _CCS_RESULT_H
#define _CCS_RESULT_H

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_error_t
ccs_create_result(ccs_objective_space_t  objective_space,
                  ccs_configuration_t    configuration,
                  ccs_error_t            error,
                  size_t                 num_values,
                  ccs_datum_t           *values,
                  void                  *user_data,
                  ccs_result_t          *result);

extern ccs_error_t
ccs_result_get_objective_space(ccs_result_t           result,
                               ccs_objective_space_t *objective_space_ret);

extern ccs_error_t
ccs_result_get_configuration(ccs_result_t         result,
                             ccs_configuration_t *configuration_ret);

extern ccs_error_t
ccs_result_get_user_data(ccs_result_t   result,
                         void         **user_data_ret);

extern ccs_error_t
ccs_result_get_error(ccs_result_t  result,
                     ccs_error_t  *error);

extern ccs_error_t
ccs_result_set_error(ccs_result_t result,
                     ccs_error_t  error);

extern ccs_error_t
ccs_result_get_value(ccs_result_t  result,
                     size_t        index,
                     ccs_datum_t  *value_ret);

extern ccs_error_t
ccs_result_set_value(ccs_result_t result,
                     size_t       index,
                     ccs_datum_t  value);

extern ccs_error_t
ccs_result_get_values(ccs_result_t  result,
                      size_t        num_values,
                      ccs_datum_t  *values,
                      size_t       *num_values_ret);

extern ccs_error_t
ccs_result_get_value_by_name(ccs_result_t  result,
                             const char   *name,
                             ccs_datum_t  *value_ret);

extern ccs_error_t
ccs_result_check(ccs_result_t  result);

#ifdef __cplusplus
}
#endif

#endif //_CCS_RESULT_H
