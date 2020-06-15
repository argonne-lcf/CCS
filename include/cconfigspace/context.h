#ifndef _CCS_CONTEXT_H
#define _CCS_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

extern ccs_error_t
ccs_context_get_hyperparameter_index(
		ccs_context_t         context,
		ccs_hyperparameter_t  hyperparameter,
		size_t               *index_ret);

#ifdef __cplusplus
}
#endif

#endif //_CCS_CONTEXT_H
