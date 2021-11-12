#ifndef _CCS_BINDING_H
#define _CCS_BINDING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file binding.h
 * A Binding is set of value in a Context see context.h.
 */

extern ccs_result_t
ccs_binding_get_context(ccs_binding_t  binding,
                        ccs_context_t *context_ret);

extern ccs_result_t
ccs_binding_get_user_data(ccs_binding_t   binding,
                          void          **user_data_ret);

extern ccs_result_t
ccs_binding_get_value(ccs_binding_t  binding,
                      size_t         index,
                      ccs_datum_t   *value_ret);

extern ccs_result_t
ccs_binding_set_value(ccs_binding_t binding,
                      size_t        index,
                      ccs_datum_t   value);

extern ccs_result_t
ccs_binding_get_values(ccs_binding_t  binding,
                       size_t         num_values,
                       ccs_datum_t   *values,
                       size_t        *num_values_ret);

extern ccs_result_t
ccs_binding_get_value_by_name(ccs_binding_t  binding,
                              const char    *name,
                              ccs_datum_t   *value_ret);

extern ccs_result_t
ccs_binding_hash(ccs_binding_t  binding,
                 ccs_hash_t    *hash_ret);

extern ccs_result_t
ccs_binding_cmp(ccs_binding_t  binding,
                ccs_binding_t  other_binding,
                int           *cmp_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_BINDING_H
