#include "cconfigspace_internal.h"
#include "binding_internal.h"

static inline _ccs_binding_ops_t *
ccs_binding_get_ops(ccs_binding_t binding)
{
	return (_ccs_binding_ops_t *)binding->obj.ops;
}

ccs_result_t
ccs_binding_get_context(ccs_binding_t binding, ccs_context_t *context_ret)
{
	CCS_CHECK_BINDING(binding);
	CCS_VALIDATE(_ccs_binding_get_context(binding, context_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_binding_get_value(
	ccs_binding_t binding,
	size_t        index,
	ccs_datum_t  *value_ret)
{
	CCS_CHECK_BINDING(binding);
	CCS_VALIDATE(_ccs_binding_get_value(binding, index, value_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_binding_get_values(
	ccs_binding_t binding,
	size_t        num_values,
	ccs_datum_t  *values,
	size_t       *num_values_ret)
{
	CCS_CHECK_BINDING(binding);
	CCS_VALIDATE(_ccs_binding_get_values(
		binding, num_values, values, num_values_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_binding_get_value_by_name(
	ccs_binding_t binding,
	const char   *name,
	ccs_datum_t  *value_ret)
{
	CCS_CHECK_BINDING(binding);
	CCS_VALIDATE(_ccs_binding_get_value_by_name(binding, name, value_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_binding_get_value_by_parameter(
	ccs_binding_t   binding,
	ccs_parameter_t parameter,
	ccs_bool_t     *found_ret,
	ccs_datum_t    *value_ret)
{
	CCS_CHECK_BINDING(binding);
	CCS_VALIDATE(_ccs_binding_get_value_by_parameter(
		binding, parameter, found_ret, value_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_binding_hash(ccs_binding_t binding, ccs_hash_t *hash_ret)
{
	CCS_CHECK_BINDING(binding);
	_ccs_binding_ops_t *ops = ccs_binding_get_ops(binding);
	CCS_VALIDATE(ops->hash(binding->data, hash_ret));
	return CCS_RESULT_SUCCESS;
}

ccs_result_t
ccs_binding_cmp(ccs_binding_t binding, ccs_binding_t other_binding, int *cmp_ret)
{
	CCS_CHECK_BINDING(binding);
	CCS_CHECK_BINDING(other_binding);
	CCS_CHECK_PTR(cmp_ret);
	if (binding == other_binding) {
		*cmp_ret = 0;
		return CCS_RESULT_SUCCESS;
	}
	_ccs_binding_ops_t *ops = ccs_binding_get_ops(binding);
	CCS_VALIDATE(ops->cmp(binding->data, other_binding, cmp_ret));
	return CCS_RESULT_SUCCESS;
}
