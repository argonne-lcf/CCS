#include "cconfigspace_internal.h"
#include "condition_internal.h"
#include <string.h>

const int ccs_expression_precedence[] = {
	0,
	1,
	2, 2,
	3, 3, 3, 3,
	4,
	5, 5,
	6, 6, 6,
	7, 7, 7,
	8
};

const char *ccs_expression_symbols[] = {
	"||",
	"&&",
	"==", "!=",
	"<", ">", "<=", ">=",
	"#",
	"+", "-",
	"*", "/", "%",
	"+", "-", "!",
	NULL
};

const int ccs_expression_arity[] = {
	2,
	2,
	2, 2,
	2, 2, 2, 2,
	2,
	2, 2,
	2, 2, 2,
	1, 1, 1,
	-1
};

static inline _ccs_expression_ops_t *
ccs_expression_get_ops(ccs_expression_t expression) {
	return (_ccs_expression_ops_t *)expression->obj.ops;
}

static ccs_error_t
_ccs_expression_del(ccs_object_t o) {
	ccs_expression_t d = (ccs_expression_t)o;
	_ccs_expression_data_t *data = d->data;
	for (size_t i = 0; i < data->num_nodes; i++)
		if (data->nodes[i].type == CCS_OBJECT)
			ccs_release_object(data->nodes[i].value.o);
	return CCS_SUCCESS;
}


static inline ccs_error_t
_ccs_expr_datum_eval(ccs_datum_t               *d,
                     ccs_configuration_space_t  context,
                     ccs_datum_t               *values,
                     ccs_datum_t               *result,
                     ccs_hyperparameter_type_t *ht) {
	ccs_object_type_t t;
	size_t index;
	ccs_error_t err;
	switch (d->type) {
	case CCS_NONE:
	case CCS_INTEGER:
	case CCS_FLOAT:
	case CCS_BOOLEAN:
	case CCS_STRING:
		*result = *d;
		break;
	case CCS_OBJECT:
		err = ccs_object_get_type(d->value.o, &t);
		if (err)
			return err;
		switch (t) {
		case CCS_EXPRESSION:
			return ccs_expression_eval((ccs_expression_t)(d->value.o),
			                           context, values, result);
			break;
		case CCS_HYPERPARAMETER:
			err = ccs_configuration_space_get_hyperparameter_index(
				context, (ccs_hyperparameter_t)(d->value.o), &index);
			if (err)
				return err;
			*result = values[index];
			if (ht) {
				err = ccs_hyperparameter_get_type(
				          (ccs_hyperparameter_t)(d->value.o), ht);
				if (err)
					return err;
			}
			break;
		default:
			return CCS_INVALID_OBJECT;
		}
		break;
	default:
		return CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

#define eval_left_right(data, context, values, left, right, htl, htr) do { \
	ccs_error_t err; \
	err = _ccs_expr_datum_eval(data->nodes, context, values, &left, htl); \
	if (err) \
		return err; \
	err = _ccs_expr_datum_eval(data->nodes + 1, context, values, &right, htr); \
	if (err) \
		return err; \
} while (0)

static ccs_error_t
_ccs_expr_or_eval(_ccs_expression_data_t *data,
                  ccs_configuration_space_t  context,
                  ccs_datum_t               *values,
                  ccs_datum_t               *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type != CCS_BOOLEAN || right.type != CCS_BOOLEAN)
		return -CCS_INVALID_VALUE;
	result->type = CCS_BOOLEAN;
	result->value.i = (left.value.i || right.value.i) ? CCS_TRUE : CCS_FALSE;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_or_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_or_eval
};

static ccs_error_t
_ccs_expr_and_eval(_ccs_expression_data_t *data,
                  ccs_configuration_space_t  context,
                  ccs_datum_t               *values,
                  ccs_datum_t               *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type != CCS_BOOLEAN || right.type != CCS_BOOLEAN)
		return -CCS_INVALID_VALUE;
	result->type = CCS_BOOLEAN;
	result->value.i = (left.value.i && right.value.i) ? CCS_TRUE : CCS_FALSE;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_and_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_and_eval
};

#define check_values(param, v) do { \
	ccs_bool_t valid; \
	ccs_error_t err; \
	err = ccs_hyperparameter_check_value( \
		(ccs_hyperparameter_t)(param), v, &valid); \
	if (unlikely(err)) \
		return err; \
	if (!valid) \
		return -CCS_INVALID_VALUE; \
} while(0)

#define check_hypers(param, v, t) do { \
	if (t == CCS_ORDINAL || t == CCS_CATEGORICAL) { \
		check_values(param.value.o, v); \
	} else if (t == CCS_NUMERICAL) {\
		if (v.type != CCS_INTEGER && v.type != CCS_FLOAT) \
			return -CCS_INVALID_VALUE; \
	} \
} while(0)

static inline ccs_int_t
_ccs_string_cmp(const char *a, const char *b) {
	if (a == b)
		return 0;
	if (!a)
		return -1;
	if (!b)
		return 1;
	return strcmp(a, b);
}

static inline ccs_error_t
_ccs_datum_test_equal_generic(ccs_datum_t *a, ccs_datum_t *b, ccs_bool_t *equal) {
	if (a->type == b->type) {
		switch(a->type) {
		case CCS_STRING:
			*equal = _ccs_string_cmp(a->value.s, b->value.s) == 0 ? CCS_TRUE : CCS_FALSE;
			break;
		case CCS_NONE:
			*equal = CCS_TRUE;
			break;
		default:
			*equal = memcmp(&(a->value), &(b->value), sizeof(ccs_value_t)) == 0 ? CCS_TRUE : CCS_FALSE;
		}
	} else {
		if (a->type == CCS_INTEGER && b->type == CCS_FLOAT) {
			*equal = (a->value.i == b->value.f) ? CCS_TRUE : CCS_FALSE;
		} else if (a->type == CCS_FLOAT && b->type == CCS_INTEGER) {
			*equal = (a->value.f == b->value.i) ? CCS_TRUE : CCS_FALSE;
		} else {
			*equal = CCS_FALSE;
			return -CCS_INVALID_VALUE;
		}
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_expr_equal_eval(_ccs_expression_data_t *data,
                  ccs_configuration_space_t  context,
                  ccs_datum_t               *values,
                  ccs_datum_t               *result) {
	ccs_datum_t               left;
	ccs_datum_t               right;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	ccs_hyperparameter_type_t htr = CCS_HYPERPARAMETER_TYPE_MAX;

	eval_left_right(data, context, values, left, right, &htl, &htr);
	check_hypers(data->nodes[0], right, htl);
	check_hypers(data->nodes[1], left, htr);
	ccs_bool_t equal;
	ccs_error_t err = _ccs_datum_test_equal_generic(&left, &right, &equal);
	if(htl != CCS_HYPERPARAMETER_TYPE_MAX || htr != CCS_HYPERPARAMETER_TYPE_MAX) {
		result->value.i = equal;
	} else {
		if (err)
			return err;
		result->value.i = equal;
	}
	result->type = CCS_BOOLEAN;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_equal_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_equal_eval
};

static inline _ccs_expression_ops_t *
_ccs_expression_ops_broker(ccs_expression_type_t  expression_type) {
	switch (expression_type) {
	case CCS_OR:
		return &_ccs_expr_or_ops;
		break;
	case CCS_AND:
		return &_ccs_expr_and_ops;
		break;
	case CCS_EQUAL:
		return &_ccs_expr_equal_ops;
		break;
	default:
		return NULL;
	}
}

ccs_error_t
ccs_create_expression(ccs_expression_type_t  expression_type,
	              size_t                 num_nodes,
                      ccs_datum_t           *nodes,
                      ccs_expression_t      *expression_ret) {
	if (num_nodes && !nodes)
		return -CCS_INVALID_VALUE;
	if (!expression_ret)
		return -CCS_INVALID_VALUE;
	int arity = ccs_expression_arity[expression_type];
	if (arity >= 0 && num_nodes != (size_t)arity)
		return -CCS_INVALID_VALUE;

	ccs_error_t err;
	size_t size_strs = 0;
	for(size_t i = 0; i < num_nodes; i++)
		if (nodes[i].type == CCS_STRING) {
			if (!nodes[i].value.s)
				return -CCS_INVALID_VALUE;
			size_strs += strlen(nodes[i].value.s) + 1;
		}

	uintptr_t mem = (uintptr_t)calloc(1,
	    sizeof(struct _ccs_expression_s) +
	    sizeof(struct _ccs_expression_data_s) +
	    num_nodes*sizeof(ccs_datum_t) +
	    size_strs);
	if (!mem)
		return -CCS_ENOMEM;

	for(size_t i = 0; i < num_nodes; i++)
		if (nodes[i].type == CCS_OBJECT) {
			err = ccs_retain_object(nodes[i].value.o);
			if (err) {
				free((void *)mem);
				return err;
			}
		}

	ccs_expression_t expression = (ccs_expression_t)mem;
	_ccs_object_init(&(expression->obj), CCS_EXPRESSION,
	                 (_ccs_object_ops_t*)_ccs_expression_ops_broker(expression_type));
	_ccs_expression_data_t *expression_data =
	    (_ccs_expression_data_t *)(mem + sizeof(struct _ccs_expression_s));
	expression_data->type = expression_type;
	expression_data->num_nodes = num_nodes;
	expression_data->nodes = (ccs_datum_t *)(mem +
	    sizeof(struct _ccs_expression_s) +
	    sizeof(struct _ccs_expression_data_s));
	char *str_pool = (char *)(mem +
	    sizeof(struct _ccs_expression_s) +
	    sizeof(struct _ccs_expression_data_s) +
	    num_nodes*sizeof(ccs_datum_t));
	for (size_t i = 0; i < num_nodes; i++) {
		if (nodes[i].type == CCS_STRING) {
			expression_data->nodes[i].type = CCS_STRING;
			expression_data->nodes[i].value.s = str_pool;
			strcpy(str_pool, nodes[i].value.s);
			str_pool += strlen(nodes[i].value.s) + 1;
		} else {
			expression_data->nodes[i] = nodes[i];
		}
	}
	expression->data = expression_data;
	*expression_ret = expression;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_expression_eval(ccs_expression_t           expression,
                    ccs_configuration_space_t  context,
                    ccs_datum_t               *values,
                    ccs_datum_t               *result) {
	if (!expression || !expression->data)
		return CCS_INVALID_OBJECT;
	if (!result)
		return CCS_INVALID_VALUE;
	_ccs_expression_ops_t *ops = ccs_expression_get_ops(expression);
	return ops->eval(expression->data, context, values, result);
}
