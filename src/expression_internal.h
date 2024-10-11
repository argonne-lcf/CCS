#ifndef _EXPRESSION_INTERNAL_H
#define _EXPRESSION_INTERNAL_H

#define CCS_EXPR_TYPE(e) ((e)->data->type)

struct _ccs_expression_data_s;
typedef struct _ccs_expression_data_s _ccs_expression_data_t;

typedef struct _ccs_expr_ctx_s {
	size_t         num_bindings;
	ccs_binding_t *bindings;
} _ccs_expr_ctx_t;

struct _ccs_expression_ops_s {
	_ccs_object_ops_t obj_ops;

	ccs_result_t (*eval)(
		ccs_expression_t expression,
		_ccs_expr_ctx_t *expr_ctx,
		ccs_datum_t     *result);
};
typedef struct _ccs_expression_ops_s _ccs_expression_ops_t;

struct _ccs_expression_s {
	_ccs_object_internal_t  obj;
	_ccs_expression_data_t *data;
};

struct _ccs_expression_data_s {
	ccs_expression_type_t type;
	size_t                num_nodes;
	ccs_expression_t     *nodes;
};

struct _ccs_expression_literal_data_s {
	_ccs_expression_data_t expr;
	ccs_datum_t            value;
};
typedef struct _ccs_expression_literal_data_s _ccs_expression_literal_data_t;

struct _ccs_expression_variable_data_s {
	_ccs_expression_data_t expr;
	ccs_parameter_t        parameter;
};
typedef struct _ccs_expression_variable_data_s _ccs_expression_variable_data_t;

struct _ccs_expression_user_defined_data_s {
	_ccs_expression_data_t               expr;
	ccs_user_defined_expression_vector_t vector;
	void                                *expression_data;
	const char                          *name;
};
typedef struct _ccs_expression_user_defined_data_s
	_ccs_expression_user_defined_data_t;

#endif //_EXPRESSION_INTERNAL_H
