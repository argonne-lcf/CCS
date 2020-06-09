#ifndef _EXPRESSION_INTERNAL_H
#define _EXPRESSION_INTERNAL_H

struct _ccs_expression_data_s;
typedef struct _ccs_expression_data_s _ccs_expression_data_t;

struct _ccs_expression_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_error_t (*eval)(
		_ccs_expression_data_t *data,
		ccs_context_t           context,
		ccs_datum_t            *values,
		ccs_datum_t            *result);
};
typedef struct _ccs_expression_ops_s _ccs_expression_ops_t;

struct _ccs_expression_s {
	_ccs_object_internal_t  obj;
	_ccs_expression_data_t *data;
};

struct _ccs_expression_data_s {
	ccs_expression_type_t  type;
	size_t                 num_nodes;
	ccs_expression_t      *nodes;
};

struct _ccs_expression_literal_data_s {
	_ccs_expression_data_t expr;
	ccs_datum_t            value;
};
typedef struct _ccs_expression_literal_data_s _ccs_expression_literal_data_t;

struct _ccs_expression_variable_data_s {
	_ccs_expression_data_t expr;
	ccs_hyperparameter_t   hyperparameter;
};
typedef struct _ccs_expression_variable_data_s _ccs_expression_variable_data_t;
#endif //_EXPRESSION_INTERNAL_H
