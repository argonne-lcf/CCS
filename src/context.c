#include "cconfigspace_internal.h"
#include "context_internal.h"

#define CCS_CHECK_CONTEXT(c) CCS_REFUTE_MSG(CCS_UNLIKELY(!(c) || !(c)->data), CCS_INVALID_OBJECT, "Invalid CCS context '%s' == %p supplied", #c, c)

ccs_error_t
ccs_context_get_hyperparameter_index(
		ccs_context_t         context,
		ccs_hyperparameter_t  hyperparameter,
		size_t               *index_ret) {
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_hyperparameter_index(
		context, hyperparameter, index_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_num_hyperparameters(
		ccs_context_t  context,
		size_t        *num_hyperparameters_ret) {
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_num_hyperparameters(context, num_hyperparameters_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_hyperparameter(
		ccs_context_t         context,
		size_t                index,
		ccs_hyperparameter_t *hyperparameter_ret) {
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_hyperparameter(context, index, hyperparameter_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_hyperparameter_by_name(
		ccs_context_t         context,
		const char *          name,
		ccs_hyperparameter_t *hyperparameter_ret) {
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_hyperparameter_by_name(context, name, hyperparameter_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_hyperparameter_index_by_name(
		ccs_context_t  context,
		const char    *name,
		size_t        *index_ret) {
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_hyperparameter_index_by_name(context, name, index_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_hyperparameters(
		ccs_context_t          context,
		size_t                 num_hyperparameters,
		ccs_hyperparameter_t  *hyperparameters,
		size_t                *num_hyperparameters_ret) {
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_hyperparameters(context, num_hyperparameters,
	                                        hyperparameters, num_hyperparameters_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_hyperparameter_indexes(
		ccs_context_t          context,
		size_t                 num_hyperparameters,
		ccs_hyperparameter_t  *hyperparameters,
		size_t                *indexes) {
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_hyperparameter_indexes(
		context, num_hyperparameters, hyperparameters, indexes));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_validate_value(ccs_context_t  context,
                           size_t         index,
                           ccs_datum_t    value,
                           ccs_datum_t   *value_ret) {
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_validate_value(context, index, value, value_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_context_get_name(ccs_context_t   context,
                     const char    **name_ret) {
	CCS_CHECK_CONTEXT(context);
	CCS_VALIDATE(_ccs_context_get_name(context, name_ret));
	return CCS_SUCCESS;
}
