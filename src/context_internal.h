#ifndef _CONTEXT_INTERNAL_H
#define _CONTEXT_INTERNAL_H

typedef struct _ccs_context_data_s _ccs_context_data_t;

struct _ccs_context_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*get_hyperparameter_index)(
		_ccs_context_data_t  *data,
		ccs_hyperparameter_t  hyperparameter,
		size_t               *index_ret);
};
typedef struct _ccs_context_ops_s _ccs_context_ops_t;

struct _ccs_context_s {
	_ccs_object_internal_t  obj;
	_ccs_context_data_t    *data;
};

#endif //_CONTEXT_INTERNAL_H
