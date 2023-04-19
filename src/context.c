#include "cconfigspace_internal.h"
#include "context_internal.h"

#define CCS_CHECK_CONTEXT(c)                                                   \
	CCS_REFUTE_MSG(                                                        \
		CCS_UNLIKELY(!(c) || !(c)->data), CCS_INVALID_OBJECT,          \
		"Invalid CCS context '%s' == %p supplied", #c, c)

ccs_error_t
ccs_context_get_parameter_index(
	ccs_context_t   context,
	ccs_parameter_t parameter,
	size_t         *index_ret)
{
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_parameter_index(
		context, parameter, index_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_num_parameters(ccs_context_t context, size_t *num_parameters_ret)
{
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(
		_ccs_context_get_num_parameters(context, num_parameters_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_parameter(
	ccs_context_t    context,
	size_t           index,
	ccs_parameter_t *parameter_ret)
{
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_parameter(context, index, parameter_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_parameter_by_name(
	ccs_context_t    context,
	const char      *name,
	ccs_parameter_t *parameter_ret)
{
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_parameter_by_name(
		context, name, parameter_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_parameter_index_by_name(
	ccs_context_t context,
	const char   *name,
	size_t       *index_ret)
{
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_parameter_index_by_name(
		context, name, index_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_parameters(
	ccs_context_t    context,
	size_t           num_parameters,
	ccs_parameter_t *parameters,
	size_t          *num_parameters_ret)
{
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_parameters(
		context, num_parameters, parameters, num_parameters_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_parameter_indexes(
	ccs_context_t    context,
	size_t           num_parameters,
	ccs_parameter_t *parameters,
	size_t          *indexes)
{
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_parameter_indexes(
		context, num_parameters, parameters, indexes));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_validate_value(
	ccs_context_t context,
	size_t        index,
	ccs_datum_t   value,
	ccs_datum_t  *value_ret)
{
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(
		_ccs_context_validate_value(context, index, value, value_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_name(ccs_context_t context, const char **name_ret)
{
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_name(context, name_ret));
	return CCS_SUCCESS;
}
