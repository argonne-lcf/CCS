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
	if (!hyperparameter)
		return -CCS_INVALID_HYPERPARAMETER;
	if (!index_ret)
		return -CCS_INVALID_VALUE;
	_ccs_context_ops_t *ops = ccs_context_get_ops(context);
	return ops->get_hyperparameter_index(context->data, hyperparameter, index_ret);
}


