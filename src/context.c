#include "cconfigspace_internal.h"
#include "context_internal.h"

static inline _ccs_context_ops_t *
ccs_context_get_ops(ccs_context_t context) {
	return (_ccs_context_ops_t *)context->obj.ops;
}

ccs_result_t
ccs_context_get_hyperparameter_index(
		ccs_context_t         context,
		ccs_hyperparameter_t  hyperparameter,
		size_t               *index_ret) {
	if (!context || !context->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_context_get_hyperparameter_index(
		context, hyperparameter, index_ret);
}

ccs_result_t
ccs_context_get_num_hyperparameters(
		ccs_context_t  context,
		size_t        *num_hyperparameters_ret) {
	if (!context || !context->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_context_get_num_hyperparameters(context, num_hyperparameters_ret);
}

ccs_result_t
ccs_context_get_hyperparameter(
		ccs_context_t         context,
		size_t                index,
		ccs_hyperparameter_t *hyperparameter_ret) {
	if (!context || !context->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_context_get_hyperparameter(context, index, hyperparameter_ret);
}

ccs_result_t
ccs_context_get_hyperparameter_by_name(
		ccs_context_t         context,
		const char *          name,
		ccs_hyperparameter_t *hyperparameter_ret) {
	if (!context || !context->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_context_get_hyperparameter_by_name(context, name, hyperparameter_ret);
}

ccs_result_t
ccs_context_get_hyperparameter_index_by_name(
		ccs_context_t  context,
		const char    *name,
		size_t        *index_ret) {
	if (!context || !context->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_context_get_hyperparameter_index_by_name(context, name, index_ret);
}

ccs_result_t
ccs_context_get_hyperparameters(
		ccs_context_t          context,
		size_t                 num_hyperparameters,
		ccs_hyperparameter_t  *hyperparameters,
		size_t                *num_hyperparameters_ret) {
	if (!context || !context->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_context_get_hyperparameters(context, num_hyperparameters,
	                                        hyperparameters, num_hyperparameters_ret);
}

ccs_result_t
ccs_context_get_hyperparameter_indexes(
		ccs_context_t          context,
		size_t                 num_hyperparameters,
		ccs_hyperparameter_t  *hyperparameters,
		size_t                *indexes) {
	if (!context || !context->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_context_get_hyperparameter_indexes(
		context, num_hyperparameters, hyperparameters, indexes);
}

ccs_result_t
ccs_context_validate_value(ccs_context_t  context,
                           size_t         index,
                           ccs_datum_t    value,
                           ccs_datum_t   *value_ret) {
	if (!context || !context->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_context_validate_value(context, index, value, value_ret);
}

ccs_result_t
ccs_context_get_name(ccs_context_t   context,
                     const char    **name_ret) {
	if (!context || !context->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_context_get_name(context, name_ret);
}

ccs_result_t
ccs_context_get_user_data(ccs_context_t   context,
                          void          **user_data_ret) {
	if (!context || !context->data)
		return -CCS_INVALID_OBJECT;
	return _ccs_context_get_user_data(context, user_data_ret);
}
