#ifndef _CONDITION_INTERNAL_H
#define _CONDITION_INTERNAL_H

struct _ccs_expression_data_s;
typedef struct _ccs_expression_data_s _ccs_expression_data_t;

struct _ccs_expression_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_error_t (*eval)(
		_ccs_expression_data_t    *data,
		ccs_configuration_space_t  context,
		ccs_datum_t               *values,
		ccs_datum_t               *result);
};
typedef struct _ccs_expression_ops_s _ccs_expression_ops_t;

struct _ccs_expression_s {
	_ccs_object_internal_t  obj;
	_ccs_expression_data_t *data;
};

struct _ccs_expression_data_s {
	ccs_expression_type_t  type;
	size_t                 num_nodes;
	ccs_datum_t           *nodes;
};

#endif //_CONDITION_INTERNAL_H
