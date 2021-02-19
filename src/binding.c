#include "cconfigspace_internal.h"
#include "binding_internal.h"

static inline _ccs_binding_ops_t *
ccs_binding_get_ops(ccs_binding_t binding) {
	return (_ccs_binding_ops_t *)binding->obj.ops;
}

ccs_result_t
ccs_binding_get_context(ccs_binding_t  binding,
                        ccs_context_t *context_ret) {
	if (!binding || !binding->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_binding_get_context(binding, context_ret);
}

ccs_result_t
ccs_binding_get_user_data(ccs_binding_t   binding,
                          void          **user_data_ret) {
	if (!binding || !binding->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_binding_get_user_data(binding, user_data_ret);
}

ccs_result_t
ccs_binding_get_value(ccs_binding_t  binding,
                      size_t         index,
                      ccs_datum_t   *value_ret) {
	if (!binding || !binding->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_binding_get_value(binding, index, value_ret);
}

ccs_result_t
ccs_binding_set_value(ccs_binding_t binding,
                      size_t        index,
                      ccs_datum_t   value) {
	if (!binding || !binding->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_binding_set_value(binding, index, value);
}

ccs_result_t
ccs_binding_get_values(ccs_binding_t  binding,
                       size_t         num_values,
                       ccs_datum_t   *values,
                       size_t        *num_values_ret) {
	if (!binding || !binding->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_binding_get_values(binding, num_values, values, num_values_ret);
}

ccs_result_t
ccs_binding_get_value_by_name(ccs_binding_t  binding,
                              const char    *name,
                              ccs_datum_t   *value_ret) {
	if (!binding || !binding->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_binding_get_value_by_name(binding, name, value_ret);
}

ccs_result_t
ccs_binding_hash(ccs_binding_t  binding,
                 ccs_hash_t    *hash_ret) {
	if (!binding || !binding->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_binding_hash(binding, hash_ret);
}

ccs_result_t
ccs_binding_cmp(ccs_binding_t  binding,
                ccs_binding_t  other_binding,
                int           *cmp_ret) {
	if (!binding || !binding->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_binding_cmp(binding, other_binding, cmp_ret);
}


