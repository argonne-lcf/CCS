#ifndef _ERROR_STACK_INTERNAL_H
#define _ERROR_STACK_INTERNAL_H
#include "utarray.h"

struct _ccs_error_stack_data_s {
	ccs_error_t  error;
	const char  *msg;
	UT_array    *elems;
};
typedef struct _ccs_error_stack_data_s _ccs_error_stack_data_t;

struct _ccs_error_stack_ops_s {
	_ccs_object_ops_t obj_ops;
};
typedef struct _ccs_error_stack_ops_s _ccs_error_stack_ops_t;

struct _ccs_error_stack_s {
	_ccs_object_internal_t   obj;
	_ccs_error_stack_data_t *data;
};

#endif //_ERROR_STACK_INTERNAL_H
