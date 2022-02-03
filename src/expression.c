#include "cconfigspace_internal.h"
#include "expression_internal.h"
#include <math.h>
#include <string.h>
#include "utarray.h"

const int ccs_expression_precedence[] = {
	0,
	1,
	2, 2,
	3, 3, 3, 3,
	4, 4,
	5, 5, 5,
	6, 6, 6,
	7,
	8,
	9, 9
};

const ccs_associativity_type_t ccs_expression_associativity[] = {
	CCS_LEFT_TO_RIGHT,
	CCS_LEFT_TO_RIGHT,
	CCS_LEFT_TO_RIGHT, CCS_LEFT_TO_RIGHT,
	CCS_LEFT_TO_RIGHT, CCS_LEFT_TO_RIGHT, CCS_LEFT_TO_RIGHT, CCS_LEFT_TO_RIGHT,
	CCS_LEFT_TO_RIGHT, CCS_LEFT_TO_RIGHT,
	CCS_LEFT_TO_RIGHT, CCS_LEFT_TO_RIGHT, CCS_LEFT_TO_RIGHT,
	CCS_RIGHT_TO_LEFT, CCS_RIGHT_TO_LEFT, CCS_RIGHT_TO_LEFT,
	CCS_LEFT_TO_RIGHT,
	CCS_LEFT_TO_RIGHT,
	CCS_ASSOCIATIVITY_TYPE_NONE, CCS_ASSOCIATIVITY_TYPE_NONE
};

const char *ccs_expression_symbols[] = {
	"||",
	"&&",
	"==", "!=",
	"<", ">", "<=", ">=",
	"+", "-",
	"*", "/", "%",
	"+", "-", "!",
	"#",
	NULL,
	NULL, NULL
};

const int ccs_expression_arity[] = {
	2,
	2,
	2, 2,
	2, 2, 2, 2,
	2, 2,
	2, 2, 2,
	1, 1, 1,
	2,
	-1,
	0, 0
};

const int ccs_terminal_precedence[] = {
	1, 1, 1,
	0, 0, 0, 0
};

const char *ccs_terminal_regexp[] = {
	"none",
	"true",
	"false",
	"\"([^\\0\\t\\n\\r\\f\"\\\\]|\\\\[0tnrf\"\\\\])+\"|'([^\\0\\t\\n\\r\\f'\\\\]|\\\\[0tnrf'\\\\])+'",
	"[a-zA-Z_][a-zA-Z_0-9]*",
	"-?[0-9]+",
	"-?[0-9]+([eE][+-]?[0-9]+|\\.[0-9]+([eE][+-]?[0-9]+)?)"
};

const char *ccs_terminal_symbols[] = {
	"none",
	"true",
	"false",
	NULL,
	NULL,
	NULL,
	NULL
};

static inline _ccs_expression_ops_t *
ccs_expression_get_ops(ccs_expression_t expression) {
	return (_ccs_expression_ops_t *)expression->obj.ops;
}

static ccs_result_t
_ccs_expression_del(ccs_object_t o) {
	ccs_expression_t d = (ccs_expression_t)o;
	_ccs_expression_data_t *data = d->data;
	for (size_t i = 0; i < data->num_nodes; i++)
		ccs_release_object(data->nodes[i]);
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_expression_data(
		_ccs_expression_data_t *data,
		size_t                 *cum_size) {
	*cum_size += _ccs_serialize_bin_size_ccs_expression_type(data->type);
	*cum_size += _ccs_serialize_bin_size_uint64(data->num_nodes);
	for (size_t i = 0; i < data->num_nodes; i++)
		CCS_VALIDATE(data->nodes[i]->obj.ops->serialize_size(
			data->nodes[i], CCS_SERIALIZE_FORMAT_BINARY, cum_size));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_expression_data(
		_ccs_expression_data_t  *data,
		size_t                  *buffer_size,
		char                   **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_type(
		data->type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_uint64(
		data->num_nodes, buffer_size, buffer));
	for (size_t i = 0; i < data->num_nodes; i++)
		CCS_VALIDATE(data->nodes[i]->obj.ops->serialize(
			data->nodes[i], CCS_SERIALIZE_FORMAT_BINARY, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_expression(
		ccs_expression_t  expression,
		size_t           *cum_size) {
	_ccs_expression_data_t *data =
		(_ccs_expression_data_t *)(expression->data);
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)expression);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_data(
		data, cum_size));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_expression(
		ccs_expression_t   expression,
		size_t            *buffer_size,
		char             **buffer) {
	_ccs_expression_data_t *data =
		(_ccs_expression_data_t *)(expression->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)expression, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_data(
		data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_expression_serialize_size(
		ccs_object_t            object,
		ccs_serialize_format_t  format,
		size_t                 *cum_size) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression(
			(ccs_expression_t)object, cum_size));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_expression_serialize(
		ccs_object_t             object,
		ccs_serialize_format_t   format,
		size_t                  *buffer_size,
		char                   **buffer) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_expression(
		    (ccs_expression_t)object, buffer_size, buffer));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_expr_node_eval(ccs_expression_t           n,
                    ccs_context_t              context,
                    ccs_datum_t               *values,
                    ccs_datum_t               *result,
                    ccs_hyperparameter_type_t *ht) {
	if (ht && n->data->type == CCS_VARIABLE) {
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)n->data;
		CCS_VALIDATE(ccs_hyperparameter_get_type(
			(ccs_hyperparameter_t)(d->hyperparameter), ht));
	}
	return ccs_expression_eval(n, context, values, result);
}

#define eval_node(data, context, values, node, ht) do { \
	CCS_VALIDATE(_ccs_expr_node_eval(data->nodes[0], context, values, &node, ht)); \
} while(0)

#define eval_left_right(data, context, values, left, right, htl, htr) do { \
	CCS_VALIDATE(_ccs_expr_node_eval(data->nodes[0], context, values, &left, htl)); \
	CCS_VALIDATE(_ccs_expr_node_eval(data->nodes[1], context, values, &right, htr)); \
} while (0)

static ccs_result_t
_ccs_expr_or_eval(_ccs_expression_data_t *data,
                  ccs_context_t           context,
                  ccs_datum_t            *values,
                  ccs_datum_t            *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	// Lazy evaluation + avoid inactive branch suppressing a hyper parameter
	// if the other branch is valid.
	ccs_result_t errl;
	ccs_result_t errr;
        errl = _ccs_expr_node_eval(data->nodes[0], context, values, &left, NULL);
        if (errl) {
		if (errl != -CCS_INACTIVE_HYPERPARAMETER)
			return errl;
	} else {
		if (left.type != CCS_BOOLEAN)
			return -CCS_INVALID_VALUE;
		if (left.value.i) {
			*result = ccs_true;
			return CCS_SUCCESS;
		}
	}
	errr = _ccs_expr_node_eval(data->nodes[1], context, values, &right, NULL);
        if (errr) {
		if (errr != -CCS_INACTIVE_HYPERPARAMETER)
			return errr;
	} else {
		if (right.type != CCS_BOOLEAN)
			return -CCS_INVALID_VALUE;
		if (right.value.i) {
                        *result = ccs_true;
			return CCS_SUCCESS;
		}
	}
	// Return inactive parameter errors
	if (errl)
		return errl;
	if (errr)
		return errr;

	*result = ccs_false;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_or_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_or_eval
};

static ccs_result_t
_ccs_expr_and_eval(_ccs_expression_data_t *data,
                  ccs_context_t            context,
                  ccs_datum_t             *values,
                  ccs_datum_t             *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type != CCS_BOOLEAN || right.type != CCS_BOOLEAN)
		return -CCS_INVALID_VALUE;
	*result = ((left.value.i && right.value.i) ? ccs_true : ccs_false);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_and_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_and_eval
};

#define check_values(param, v) do { \
	ccs_bool_t valid; \
	CCS_VALIDATE(ccs_hyperparameter_check_value(param, v, &valid)); \
	if (!valid) \
		return -CCS_INVALID_VALUE; \
} while(0)

#define check_hypers(param, v, t) do { \
	if (t == CCS_HYPERPARAMETER_TYPE_ORDINAL || t == CCS_HYPERPARAMETER_TYPE_CATEGORICAL) { \
		_ccs_expression_variable_data_t *d = \
			(_ccs_expression_variable_data_t *)param->data; \
		check_values(d->hyperparameter, v); \
	} else if (t == CCS_HYPERPARAMETER_TYPE_NUMERICAL || t == CCS_HYPERPARAMETER_TYPE_DISCRETE) {\
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

static inline ccs_result_t
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

static inline ccs_result_t
_ccs_datum_cmp_generic(ccs_datum_t *a, ccs_datum_t *b, ccs_int_t *cmp) {
	if (a->type == b->type) {
		switch(a->type) {
		case CCS_STRING:
			*cmp = _ccs_string_cmp(a->value.s, b->value.s);
			break;
		case CCS_INTEGER:
			*cmp = a->value.i < b->value.i ? -1 :
			       a->value.i > b->value.i ?  1 : 0;
			break;
		case CCS_FLOAT:
			*cmp = a->value.f < b->value.f ? -1 :
			       a->value.f > b->value.f ?  1 : 0;
			break;
		default:
			return -CCS_INVALID_VALUE;
		}
	} else {
		if (a->type == CCS_INTEGER && b->type == CCS_FLOAT) {
			*cmp = a->value.i < b->value.f ? -1 :
			       a->value.i > b->value.f ?  1 : 0;
		} else if (a->type == CCS_FLOAT && b->type == CCS_INTEGER) {
			*cmp = a->value.f < b->value.i ? -1 :
			       a->value.f > b->value.i ?  1 : 0;
		} else {
			return -CCS_INVALID_VALUE;
		}
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_expr_equal_eval(_ccs_expression_data_t *data,
                     ccs_context_t           context,
                     ccs_datum_t            *values,
                     ccs_datum_t            *result) {
	ccs_datum_t               left;
	ccs_datum_t               right;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	ccs_hyperparameter_type_t htr = CCS_HYPERPARAMETER_TYPE_MAX;

	eval_left_right(data, context, values, left, right, &htl, &htr);
	check_hypers(data->nodes[0], right, htl);
	check_hypers(data->nodes[1], left, htr);
	ccs_bool_t equal;
	ccs_result_t err = _ccs_datum_test_equal_generic(&left, &right, &equal);
	if (htl == CCS_HYPERPARAMETER_TYPE_MAX &&
	    htr == CCS_HYPERPARAMETER_TYPE_MAX &&
	    err)
		return err;
	*result = (equal ? ccs_true : ccs_false);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_equal_ops = {
	{ &_ccs_expression_del, NULL, NULL },
	&_ccs_expr_equal_eval
};

static ccs_result_t
_ccs_expr_not_equal_eval(_ccs_expression_data_t *data,
                         ccs_context_t           context,
                         ccs_datum_t            *values,
                         ccs_datum_t            *result) {
	ccs_datum_t               left;
	ccs_datum_t               right;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	ccs_hyperparameter_type_t htr = CCS_HYPERPARAMETER_TYPE_MAX;

	eval_left_right(data, context, values, left, right, &htl, &htr);
	check_hypers(data->nodes[0], right, htl);
	check_hypers(data->nodes[1], left, htr);
	ccs_bool_t equal;
	ccs_result_t err = _ccs_datum_test_equal_generic(&left, &right, &equal);
	if(htl != CCS_HYPERPARAMETER_TYPE_MAX || htr != CCS_HYPERPARAMETER_TYPE_MAX) {
		*result = (equal ? ccs_false : ccs_true);
	} else {
		if (err)
			return err;
		*result = (equal ? ccs_false : ccs_true);
	}
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_not_equal_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_not_equal_eval
};

static ccs_result_t
_ccs_expr_less_eval(_ccs_expression_data_t *data,
                    ccs_context_t           context,
                    ccs_datum_t            *values,
                    ccs_datum_t            *result) {
	ccs_datum_t               left;
	ccs_datum_t               right;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	ccs_hyperparameter_type_t htr = CCS_HYPERPARAMETER_TYPE_MAX;

	eval_left_right(data, context, values, left, right, &htl, &htr);
	if (htl == CCS_HYPERPARAMETER_TYPE_CATEGORICAL || htr == CCS_HYPERPARAMETER_TYPE_CATEGORICAL)
		return -CCS_INVALID_VALUE;
	check_hypers(data->nodes[0], right, htl);
	check_hypers(data->nodes[1], left, htr);
	if (htl == CCS_HYPERPARAMETER_TYPE_ORDINAL) {
		ccs_int_t cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[0]->data;
		CCS_VALIDATE(ccs_ordinal_hyperparameter_compare_values(
		    d->hyperparameter, left, right, &cmp));
		*result = (cmp < 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	if (htr == CCS_HYPERPARAMETER_TYPE_ORDINAL) {
		ccs_int_t cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[1]->data;
		CCS_VALIDATE(ccs_ordinal_hyperparameter_compare_values(
		    d->hyperparameter, left, right, &cmp));
		*result = (cmp < 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	ccs_int_t cmp;
	CCS_VALIDATE(_ccs_datum_cmp_generic(&left, &right, &cmp));
	*result = (cmp < 0 ? ccs_true : ccs_false);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_less_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_less_eval
};

static ccs_result_t
_ccs_expr_greater_eval(_ccs_expression_data_t *data,
                       ccs_context_t           context,
                       ccs_datum_t            *values,
                       ccs_datum_t            *result) {
	ccs_datum_t               left;
	ccs_datum_t               right;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	ccs_hyperparameter_type_t htr = CCS_HYPERPARAMETER_TYPE_MAX;

	eval_left_right(data, context, values, left, right, &htl, &htr);
	if (htl == CCS_HYPERPARAMETER_TYPE_CATEGORICAL || htr == CCS_HYPERPARAMETER_TYPE_CATEGORICAL)
		return -CCS_INVALID_VALUE;
	check_hypers(data->nodes[0], right, htl);
	check_hypers(data->nodes[1], left, htr);
	if (htl == CCS_HYPERPARAMETER_TYPE_ORDINAL) {
		ccs_int_t cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[0]->data;
		CCS_VALIDATE(ccs_ordinal_hyperparameter_compare_values(
		    d->hyperparameter, left, right, &cmp));
		*result = (cmp > 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	if (htr == CCS_HYPERPARAMETER_TYPE_ORDINAL) {
		ccs_int_t cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[1]->data;
		CCS_VALIDATE(ccs_ordinal_hyperparameter_compare_values(
		    d->hyperparameter, left, right, &cmp));
		*result = (cmp > 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	ccs_int_t cmp;
	CCS_VALIDATE(_ccs_datum_cmp_generic(&left, &right, &cmp));
	*result = (cmp > 0 ? ccs_true : ccs_false);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_greater_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_greater_eval
};

static ccs_result_t
_ccs_expr_less_or_equal_eval(_ccs_expression_data_t *data,
                             ccs_context_t           context,
                             ccs_datum_t            *values,
                             ccs_datum_t            *result) {
	ccs_datum_t               left;
	ccs_datum_t               right;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	ccs_hyperparameter_type_t htr = CCS_HYPERPARAMETER_TYPE_MAX;

	eval_left_right(data, context, values, left, right, &htl, &htr);
	if (htl == CCS_HYPERPARAMETER_TYPE_CATEGORICAL || htr == CCS_HYPERPARAMETER_TYPE_CATEGORICAL)
		return -CCS_INVALID_VALUE;
	check_hypers(data->nodes[0], right, htl);
	check_hypers(data->nodes[1], left, htr);
	if (htl == CCS_HYPERPARAMETER_TYPE_ORDINAL) {
		ccs_int_t cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[0]->data;
		CCS_VALIDATE(ccs_ordinal_hyperparameter_compare_values(
		    d->hyperparameter, left, right, &cmp));
		*result = (cmp <= 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	if (htr == CCS_HYPERPARAMETER_TYPE_ORDINAL) {
		ccs_int_t cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[1]->data;
		CCS_VALIDATE(ccs_ordinal_hyperparameter_compare_values(
		    d->hyperparameter, left, right, &cmp));
		*result = (cmp <= 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	ccs_int_t cmp;
	CCS_VALIDATE(_ccs_datum_cmp_generic(&left, &right, &cmp));
	*result = (cmp <= 0 ? ccs_true : ccs_false);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_less_or_equal_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_less_or_equal_eval
};

static ccs_result_t
_ccs_expr_greater_or_equal_eval(_ccs_expression_data_t *data,
                                ccs_context_t           context,
                                ccs_datum_t            *values,
                                ccs_datum_t            *result) {
	ccs_datum_t               left;
	ccs_datum_t               right;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	ccs_hyperparameter_type_t htr = CCS_HYPERPARAMETER_TYPE_MAX;

	eval_left_right(data, context, values, left, right, &htl, &htr);
	if (htl == CCS_HYPERPARAMETER_TYPE_CATEGORICAL || htr == CCS_HYPERPARAMETER_TYPE_CATEGORICAL)
		return -CCS_INVALID_VALUE;
	check_hypers(data->nodes[0], right, htl);
	check_hypers(data->nodes[1], left, htr);
	if (htl == CCS_HYPERPARAMETER_TYPE_ORDINAL) {
		ccs_int_t cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[0]->data;
		CCS_VALIDATE(ccs_ordinal_hyperparameter_compare_values(
		    d->hyperparameter, left, right, &cmp));
		*result = (cmp >= 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	if (htr == CCS_HYPERPARAMETER_TYPE_ORDINAL) {
		ccs_int_t cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[1]->data;
		CCS_VALIDATE(ccs_ordinal_hyperparameter_compare_values(
		    d->hyperparameter, left, right, &cmp));
		*result = (cmp >= 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	ccs_int_t cmp;
	CCS_VALIDATE(_ccs_datum_cmp_generic(&left, &right, &cmp));
	*result = (cmp >= 0 ? ccs_true : ccs_false);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_greater_or_equal_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_greater_or_equal_eval
};

static ccs_result_t
_ccs_expr_in_eval(_ccs_expression_data_t *data,
                  ccs_context_t           context,
                  ccs_datum_t            *values,
                  ccs_datum_t            *result) {
	ccs_expression_type_t etype;
	CCS_VALIDATE(ccs_expression_get_type(data->nodes[1], &etype));
	if (etype != CCS_LIST)
		return -CCS_INVALID_VALUE;
	size_t num_nodes;
	CCS_VALIDATE(ccs_expression_get_num_nodes(data->nodes[1], &num_nodes));
	if (num_nodes == 0) {
		*result = ccs_false;
		return CCS_SUCCESS;
	}

	ccs_datum_t               left;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	eval_node(data, context, values, left, &htl);
	for (size_t i = 0; i < num_nodes; i++) {
		ccs_datum_t right;
		CCS_VALIDATE(ccs_expression_list_eval_node(data->nodes[1], context, values, i, &right));
		check_hypers(data->nodes[0], right, htl);
		ccs_bool_t equal;
		ccs_result_t err = _ccs_datum_test_equal_generic(&left, &right, &equal);
		if (err) {
			if (err != -CCS_INVALID_VALUE)
				return err;
			else
				continue;
		}
		if (equal) {
			*result = ccs_true;
			return CCS_SUCCESS;
		}
	}
	*result = ccs_false;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_in_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_in_eval
};

static ccs_result_t
_ccs_expr_add_eval(_ccs_expression_data_t *data,
                   ccs_context_t           context,
                   ccs_datum_t            *values,
                   ccs_datum_t            *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			*result = ccs_int(left.value.i + right.value.i);
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			*result = ccs_float(left.value.i + right.value.f);
			return CCS_SUCCESS;
		}
		return -CCS_INVALID_VALUE;
	}
	if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			*result = ccs_float(left.value.f + right.value.i);
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			*result = ccs_float(left.value.f + right.value.f);
			return CCS_SUCCESS;
		}
	}
	return -CCS_INVALID_VALUE;
}

static _ccs_expression_ops_t _ccs_expr_add_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_add_eval
};

static ccs_result_t
_ccs_expr_substract_eval(_ccs_expression_data_t *data,
                         ccs_context_t           context,
                         ccs_datum_t            *values,
                         ccs_datum_t            *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			*result = ccs_int(left.value.i - right.value.i);
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			*result = ccs_float(left.value.i - right.value.f);
			return CCS_SUCCESS;
		}
		return -CCS_INVALID_VALUE;
	}
	if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			*result = ccs_float(left.value.f - right.value.i);
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			*result = ccs_float(left.value.f - right.value.f);
			return CCS_SUCCESS;
		}
	}
	return -CCS_INVALID_VALUE;
}

static _ccs_expression_ops_t _ccs_expr_substract_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_substract_eval
};

static ccs_result_t
_ccs_expr_multiply_eval(_ccs_expression_data_t *data,
                        ccs_context_t           context,
                        ccs_datum_t            *values,
                        ccs_datum_t            *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			*result = ccs_int(left.value.i * right.value.i);
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			*result = ccs_float(left.value.i * right.value.f);
			return CCS_SUCCESS;
		}
		return -CCS_INVALID_VALUE;
	}
	if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			*result = ccs_float(left.value.f * right.value.i);
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			*result = ccs_float(left.value.f * right.value.f);
			return CCS_SUCCESS;
		}
	}
	return -CCS_INVALID_VALUE;
}

static _ccs_expression_ops_t _ccs_expr_multiply_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_multiply_eval
};

static ccs_result_t
_ccs_expr_divide_eval(_ccs_expression_data_t *data,
                      ccs_context_t           context,
                      ccs_datum_t            *values,
                      ccs_datum_t            *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			if (right.value.i == 0)
				return -CCS_INVALID_VALUE;
			*result = ccs_int(left.value.i / right.value.i);
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			if (right.value.f == 0.0)
				return -CCS_INVALID_VALUE;
			*result = ccs_float(left.value.i / right.value.f);
			return CCS_SUCCESS;
		}
		return -CCS_INVALID_VALUE;
	}
	if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			if (right.value.i == 0)
				return -CCS_INVALID_VALUE;
			*result = ccs_float(left.value.f / right.value.i);
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			if (right.value.f == 0.0)
				return -CCS_INVALID_VALUE;
			*result = ccs_float(left.value.f / right.value.f);
			return CCS_SUCCESS;
		}
	}
	return -CCS_INVALID_VALUE;
}

static _ccs_expression_ops_t _ccs_expr_divide_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_divide_eval
};

static ccs_result_t
_ccs_expr_modulo_eval(_ccs_expression_data_t *data,
                      ccs_context_t           context,
                      ccs_datum_t            *values,
                      ccs_datum_t            *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			if (right.value.i == 0)
				return -CCS_INVALID_VALUE;
			*result = ccs_int(left.value.i % right.value.i);
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			if (right.value.f == 0.0)
				return -CCS_INVALID_VALUE;
			*result = ccs_float(fmod(left.value.i, right.value.f));
			return CCS_SUCCESS;
		}
		return -CCS_INVALID_VALUE;
	}
	if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			if (right.value.i == 0)
				return -CCS_INVALID_VALUE;
			*result = ccs_float(fmod(left.value.f, right.value.i));
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			if (right.value.f == 0.0)
				return -CCS_INVALID_VALUE;
			*result = ccs_float(fmod(left.value.f, right.value.f));
			return CCS_SUCCESS;
		}
	}
	return -CCS_INVALID_VALUE;
}

static _ccs_expression_ops_t _ccs_expr_modulo_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_modulo_eval
};

static ccs_result_t
_ccs_expr_positive_eval(_ccs_expression_data_t *data,
                        ccs_context_t           context,
                        ccs_datum_t            *values,
                        ccs_datum_t            *result) {
	ccs_datum_t node;
	eval_node(data, context, values, node, NULL);
	if (node.type != CCS_INTEGER && node.type != CCS_FLOAT) {
		return -CCS_INVALID_VALUE;
	}
	*result = node;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_positive_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_positive_eval
};

static ccs_result_t
_ccs_expr_negative_eval(_ccs_expression_data_t *data,
                        ccs_context_t           context,
                        ccs_datum_t            *values,
                        ccs_datum_t            *result) {
	ccs_datum_t node;
	eval_node(data, context, values, node, NULL);
	if (node.type == CCS_INTEGER) {
		*result = ccs_int(- node.value.i);
	} else if (node.type == CCS_FLOAT) {
		*result = ccs_float(- node.value.f);
	} else
		return -CCS_INVALID_VALUE;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_negative_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_negative_eval
};

static ccs_result_t
_ccs_expr_not_eval(_ccs_expression_data_t *data,
                   ccs_context_t           context,
                   ccs_datum_t            *values,
                   ccs_datum_t            *result) {
	ccs_datum_t node;
	eval_node(data, context, values, node, NULL);
	if (node.type == CCS_BOOLEAN) {
		*result = (node.value.i ? ccs_false : ccs_true);
	} else
		return -CCS_INVALID_VALUE;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_not_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_not_eval
};

static ccs_result_t
_ccs_expr_list_eval(_ccs_expression_data_t *data,
                    ccs_context_t           context,
                    ccs_datum_t            *values,
                    ccs_datum_t            *result) {
	(void)data; (void)context; (void)values; (void)result;
	return -CCS_UNSUPPORTED_OPERATION;
}

static _ccs_expression_ops_t _ccs_expr_list_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_serialize_size,
	  &_ccs_expression_serialize },
	&_ccs_expr_list_eval
};

static ccs_result_t
_ccs_expr_literal_eval(_ccs_expression_data_t *data,
                       ccs_context_t           context,
                       ccs_datum_t            *values,
                       ccs_datum_t            *result) {
	(void)context; (void)values;
	_ccs_expression_literal_data_t *d =
		(_ccs_expression_literal_data_t *)data;
	*result = d->value;
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_expression_literal_data(
		_ccs_expression_literal_data_t *data,
		size_t                         *cum_size) {
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_data(
		&data->expr, cum_size));
	*cum_size += _ccs_serialize_bin_size_ccs_datum(data->value);
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_expression_literal_data(
		_ccs_expression_literal_data_t  *data,
		size_t                          *buffer_size,
		char                           **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_data(
		&data->expr, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_datum(
		data->value, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_expression_literal(
		ccs_expression_t  expression,
		size_t           *cum_size) {
	_ccs_expression_literal_data_t *data =
		(_ccs_expression_literal_data_t *)(expression->data);
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)expression);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_literal_data(
		data, cum_size));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_expression_literal(
		ccs_expression_t   expression,
		size_t            *buffer_size,
		char             **buffer) {
	_ccs_expression_literal_data_t *data =
		(_ccs_expression_literal_data_t *)(expression->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)expression, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_literal_data(
		data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_expression_literal_serialize_size(
		ccs_object_t            object,
		ccs_serialize_format_t  format,
		size_t                 *cum_size) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_literal(
			(ccs_expression_t)object, cum_size));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_expression_literal_serialize(
		ccs_object_t             object,
		ccs_serialize_format_t   format,
		size_t                  *buffer_size,
		char                   **buffer) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_literal(
		    (ccs_expression_t)object, buffer_size, buffer));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_literal_ops = {
	{ &_ccs_expression_del,
	  &_ccs_expression_literal_serialize_size,
	  &_ccs_expression_literal_serialize },
	&_ccs_expr_literal_eval
};

static ccs_result_t
_ccs_expr_variable_del(ccs_object_t o) {
	_ccs_expression_variable_data_t *d =
		(_ccs_expression_variable_data_t *)((ccs_expression_t)o)->data;
	return ccs_release_object(d->hyperparameter);
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_expression_variable_data(
		_ccs_expression_variable_data_t *data,
		size_t                         *cum_size) {
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_data(
		&data->expr, cum_size));
	*cum_size += _ccs_serialize_bin_size_ccs_object(data->hyperparameter);
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_expression_variable_data(
		_ccs_expression_variable_data_t  *data,
		size_t                           *buffer_size,
		char                            **buffer) {
	CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_data(
		&data->expr, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object(
		data->hyperparameter, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_size_ccs_expression_variable(
		ccs_expression_t  expression,
		size_t           *cum_size) {
	_ccs_expression_variable_data_t *data =
		(_ccs_expression_variable_data_t *)(expression->data);
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)expression);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_variable_data(
		data, cum_size));
	return CCS_SUCCESS;
}

static inline ccs_result_t
_ccs_serialize_bin_ccs_expression_variable(
		ccs_expression_t   expression,
		size_t            *buffer_size,
		char             **buffer) {
	_ccs_expression_variable_data_t *data =
		(_ccs_expression_variable_data_t *)(expression->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)expression, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_variable_data(
		data, buffer_size, buffer));
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_expression_variable_serialize_size(
		ccs_object_t            object,
		ccs_serialize_format_t  format,
		size_t                 *cum_size) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_variable(
			(ccs_expression_t)object, cum_size));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_expression_variable_serialize(
		ccs_object_t             object,
		ccs_serialize_format_t   format,
		size_t                  *buffer_size,
		char                   **buffer) {
	switch(format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_variable(
			(ccs_expression_t)object, buffer_size, buffer));
		break;
	default:
		return -CCS_INVALID_VALUE;
	}
	return CCS_SUCCESS;
}

static ccs_result_t
_ccs_expr_variable_eval(_ccs_expression_data_t *data,
                        ccs_context_t           context,
                        ccs_datum_t            *values,
                        ccs_datum_t            *result) {
	_ccs_expression_variable_data_t *d =
		(_ccs_expression_variable_data_t *)data;
	size_t index;
	CCS_CHECK_PTR(values);
	CCS_VALIDATE(ccs_context_get_hyperparameter_index(context,
	    (ccs_hyperparameter_t)(d->hyperparameter), &index));
	*result = values[index];
	if (result->type == CCS_INACTIVE)
		return -CCS_INACTIVE_HYPERPARAMETER;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_variable_ops = {
	{ &_ccs_expr_variable_del,
	  &_ccs_expression_variable_serialize_size,
	  &_ccs_expression_variable_serialize },
	&_ccs_expr_variable_eval
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
	case CCS_NOT_EQUAL:
		return &_ccs_expr_not_equal_ops;
		break;
	case CCS_LESS:
		return &_ccs_expr_less_ops;
		break;
	case CCS_GREATER:
		return &_ccs_expr_greater_ops;
		break;
	case CCS_LESS_OR_EQUAL:
		return &_ccs_expr_less_or_equal_ops;
		break;
	case CCS_GREATER_OR_EQUAL:
		return &_ccs_expr_greater_or_equal_ops;
		break;
	case CCS_ADD:
		return &_ccs_expr_add_ops;
		break;
	case CCS_SUBSTRACT:
		return &_ccs_expr_substract_ops;
		break;
	case CCS_MULTIPLY:
		return &_ccs_expr_multiply_ops;
		break;
	case CCS_DIVIDE:
		return &_ccs_expr_divide_ops;
		break;
	case CCS_MODULO:
		return &_ccs_expr_modulo_ops;
		break;
	case CCS_POSITIVE:
		return &_ccs_expr_positive_ops;
		break;
	case CCS_NEGATIVE:
		return &_ccs_expr_negative_ops;
		break;
	case CCS_NOT:
		return &_ccs_expr_not_ops;
		break;
	case CCS_IN:
		return &_ccs_expr_in_ops;
		break;
	case CCS_LIST:
		return &_ccs_expr_list_ops;
		break;
	case CCS_LITERAL:
		return &_ccs_expr_literal_ops;
		break;
	case CCS_VARIABLE:
		return &_ccs_expr_variable_ops;
		break;
	default:
		return NULL;
	}
}

ccs_result_t
ccs_create_literal(ccs_datum_t       value,
                   ccs_expression_t *expression_ret) {
	if (value.type < CCS_NONE || value.type > CCS_STRING)
		return -CCS_INVALID_VALUE;
	CCS_CHECK_PTR(expression_ret);
	size_t size_str = 0;
	if (value.type == CCS_STRING && value.value.s) {
		size_str = strlen(value.value.s) + 1;
	}
	uintptr_t mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_expression_s) +
		sizeof(struct _ccs_expression_literal_data_s) +
		size_str);
	if(!mem)
		return -CCS_OUT_OF_MEMORY;
	ccs_expression_t expression = (ccs_expression_t)mem;
	_ccs_object_init(&(expression->obj), CCS_EXPRESSION, NULL,
		(_ccs_object_ops_t*)_ccs_expression_ops_broker(CCS_LITERAL));
	_ccs_expression_literal_data_t *expression_data =
		 (_ccs_expression_literal_data_t *)
			(mem + sizeof(struct _ccs_expression_s));
	expression_data->expr.type = CCS_LITERAL;
	expression_data->expr.num_nodes = 0;
	expression_data->expr.nodes = NULL;
	if (size_str) {
		char *str_pool = (char *)(mem +
			sizeof(struct _ccs_expression_s) +
			sizeof(struct _ccs_expression_literal_data_s));
		expression_data->value = ccs_string(str_pool);
		strcpy(str_pool, value.value.s);
	} else {
		expression_data->value = value;
		expression_data->value.flags = CCS_FLAG_DEFAULT;
	}
	expression->data = (_ccs_expression_data_t *)expression_data;
	*expression_ret = expression;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_create_variable(ccs_hyperparameter_t  hyperparameter,
                    ccs_expression_t     *expression_ret) {
	CCS_CHECK_OBJ(hyperparameter, CCS_HYPERPARAMETER);
	CCS_CHECK_PTR(expression_ret);
	ccs_result_t err;
	uintptr_t mem = (uintptr_t)calloc(1,
		sizeof(struct _ccs_expression_s) +
		sizeof(struct _ccs_expression_variable_data_s));
	if (!mem)
		return -CCS_OUT_OF_MEMORY;
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(hyperparameter), errmem);
	ccs_expression_t expression;
	expression = (ccs_expression_t)mem;
	_ccs_object_init(&(expression->obj), CCS_EXPRESSION, NULL,
		(_ccs_object_ops_t*)_ccs_expression_ops_broker(CCS_VARIABLE));
	_ccs_expression_variable_data_t *expression_data;
	expression_data =
		(_ccs_expression_variable_data_t *)
			(mem + sizeof(struct _ccs_expression_s));
	expression_data->expr.type = CCS_VARIABLE;
	expression_data->expr.num_nodes = 0;
	expression_data->expr.nodes = NULL;
	expression_data->hyperparameter = hyperparameter;
	expression->data = (_ccs_expression_data_t *)expression_data;
	*expression_ret = expression;
	return CCS_SUCCESS;
errmem:
	free((void *)mem);
	return err;
}

ccs_result_t
ccs_create_expression(ccs_expression_type_t  type,
	              size_t                 num_nodes,
                      ccs_datum_t           *nodes,
                      ccs_expression_t      *expression_ret) {
	CCS_CHECK_ARY(num_nodes, nodes);
	CCS_CHECK_PTR(expression_ret);
	if (type < CCS_OR || type > CCS_LIST)
		return -CCS_INVALID_VALUE;
	int arity = ccs_expression_arity[type];
	if (arity >= 0 && num_nodes != (size_t)arity)
		return -CCS_INVALID_VALUE;
	ccs_result_t err;
	for(size_t i = 0; i < num_nodes; i++){
		if (nodes[i].type == CCS_OBJECT) {
			ccs_object_type_t t;
			CCS_VALIDATE(ccs_object_get_type(nodes[i].value.o, &t));
			if (t != CCS_HYPERPARAMETER && t != CCS_EXPRESSION)
				return -CCS_INVALID_VALUE;
		} else if (nodes[i].type < CCS_NONE || nodes[i].type > CCS_STRING)
			return -CCS_INVALID_VALUE;
	}

	uintptr_t mem = (uintptr_t)calloc(1,
	    sizeof(struct _ccs_expression_s) +
	    sizeof(struct _ccs_expression_data_s) +
	    num_nodes*sizeof(ccs_expression_t));
	if (!mem)
		return -CCS_OUT_OF_MEMORY;

	ccs_expression_t expression = (ccs_expression_t)mem;
	_ccs_object_init(&(expression->obj), CCS_EXPRESSION, NULL,
	                 (_ccs_object_ops_t*)_ccs_expression_ops_broker(type));
	_ccs_expression_data_t *expression_data =
	    (_ccs_expression_data_t *)(mem + sizeof(struct _ccs_expression_s));
	expression_data->type = type;
	expression_data->num_nodes = num_nodes;
	expression_data->nodes = (ccs_expression_t *)(mem +
	    sizeof(struct _ccs_expression_s) +
	    sizeof(struct _ccs_expression_data_s));
	for (size_t i = 0; i < num_nodes; i++) {
		if (nodes[i].type == CCS_OBJECT) {
			ccs_object_type_t t;
			CCS_VALIDATE_ERR_GOTO(err,
			    ccs_object_get_type(nodes[i].value.o, &t), cleanup);
			if (t == CCS_EXPRESSION) {
				CCS_VALIDATE_ERR_GOTO(err,
				    ccs_retain_object(nodes[i].value.o), cleanup);
				expression_data->nodes[i] =
					(ccs_expression_t)nodes[i].value.o;
			} else {
				CCS_VALIDATE_ERR_GOTO(err, ccs_create_variable(
					(ccs_hyperparameter_t)nodes[i].value.o,
					expression_data->nodes + i), cleanup);
			}
		} else {
			CCS_VALIDATE_ERR_GOTO(err, ccs_create_literal(
			    nodes[i], expression_data->nodes + i), cleanup);
		}
	}
	expression->data = expression_data;
	*expression_ret = expression;
	return CCS_SUCCESS;
cleanup:
	for (size_t i = 0; i < num_nodes; i++) {
		if (expression_data->nodes[i])
			ccs_release_object(expression_data->nodes[i]);
	}
	free((void*)mem);
	return err;
}

ccs_result_t
ccs_create_binary_expression(ccs_expression_type_t  type,
                             ccs_datum_t            node_left,
                             ccs_datum_t            node_right,
                             ccs_expression_t      *expression_ret) {
	ccs_datum_t nodes[2];
	nodes[0] = node_left;
	nodes[1] = node_right;
	return ccs_create_expression(type, 2, nodes, expression_ret);
}

ccs_result_t
ccs_create_unary_expression(ccs_expression_type_t  type,
                            ccs_datum_t            node,
                            ccs_expression_t      *expression_ret) {
	return ccs_create_expression(type, 1, &node, expression_ret);
}

ccs_result_t
ccs_expression_eval(ccs_expression_t  expression,
                    ccs_context_t     context,
                    ccs_datum_t      *values,
                    ccs_datum_t      *result_ret) {
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	CCS_CHECK_PTR(result_ret);
	_ccs_expression_ops_t *ops = ccs_expression_get_ops(expression);
	return ops->eval(expression->data, context, values, result_ret);
}

ccs_result_t
ccs_expression_get_num_nodes(ccs_expression_t  expression,
                             size_t           *num_nodes_ret) {
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	CCS_CHECK_PTR(num_nodes_ret);
	*num_nodes_ret = expression->data->num_nodes;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_expression_get_nodes(ccs_expression_t  expression,
                         size_t            num_nodes,
                         ccs_expression_t *nodes,
                         size_t           *num_nodes_ret) {
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	CCS_CHECK_ARY(num_nodes, nodes);
	if (!num_nodes_ret && !nodes)
		return -CCS_INVALID_VALUE;
	size_t count = expression->data->num_nodes;
	if (nodes) {
		ccs_expression_t *p_nodes = expression->data->nodes;
		if (num_nodes < count)
			return -CCS_INVALID_VALUE;
		for (size_t i = 0; i < count; i++)
			nodes[i] = p_nodes[i];
		for (size_t i = count; i < num_nodes; i++)
			nodes[i] = NULL;
	}
	if (num_nodes_ret)
		*num_nodes_ret = count;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_expression_list_eval_node(ccs_expression_t  expression,
                              ccs_context_t     context,
                              ccs_datum_t      *values,
                              size_t            index,
                              ccs_datum_t      *result) {
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	CCS_CHECK_PTR(result);
	if (expression->data->type != CCS_LIST)
		return -CCS_INVALID_EXPRESSION;
	ccs_datum_t node;
	if (index >= expression->data->num_nodes)
		return -CCS_OUT_OF_BOUNDS;
	CCS_VALIDATE(_ccs_expr_node_eval(expression->data->nodes[index], context, values, &node, NULL));
	*result = node;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_expression_get_type(ccs_expression_t       expression,
                        ccs_expression_type_t *type_ret) {
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	CCS_CHECK_PTR(type_ret);
	*type_ret = expression->data->type;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_literal_get_value(ccs_expression_t  expression,
                      ccs_datum_t      *value_ret) {
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	CCS_CHECK_PTR(value_ret);
	if (expression->data->type != CCS_LITERAL)
		return -CCS_INVALID_EXPRESSION;
	_ccs_expression_literal_data_t *d =
		(_ccs_expression_literal_data_t *)expression->data;
	*value_ret = d->value;
	return CCS_SUCCESS;
}

ccs_result_t
ccs_variable_get_hyperparameter(ccs_expression_t      expression,
                                ccs_hyperparameter_t *hyperparameter_ret) {
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	CCS_CHECK_PTR(hyperparameter_ret);
	if (expression->data->type != CCS_VARIABLE)
		return -CCS_INVALID_EXPRESSION;
	_ccs_expression_variable_data_t *d =
		(_ccs_expression_variable_data_t *)expression->data;
	*hyperparameter_ret = d->hyperparameter;
	return CCS_SUCCESS;
}

#undef  utarray_oom
#define utarray_oom() { \
	return -CCS_OUT_OF_MEMORY; \
}

static ccs_result_t _get_hyperparameters(ccs_expression_t  expression,
                                         UT_array         *array) {
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	if (expression->data->type == CCS_VARIABLE) {
		_ccs_expression_variable_data_t * d =
			(_ccs_expression_variable_data_t *)expression->data;
		utarray_push_back(array, &(d->hyperparameter));
	} else
		for (size_t i = 0; i < expression->data->num_nodes; i++)
			CCS_VALIDATE(_get_hyperparameters(expression->data->nodes[i], array));
	return CCS_SUCCESS;
}

static const UT_icd _hyperparameter_icd = {
	sizeof(ccs_hyperparameter_t),
	NULL,
	NULL,
	NULL,
};

static int _hyper_sort(const void *a, const void *b) {
	ccs_hyperparameter_t ha = *(ccs_hyperparameter_t *)a;
	ccs_hyperparameter_t hb = *(ccs_hyperparameter_t *)b;
	return ha < hb ? -1 : ha > hb ? 1 : 0;
}

ccs_result_t
ccs_expression_get_hyperparameters(ccs_expression_t      expression,
                                   size_t                num_hyperparameters,
                                   ccs_hyperparameter_t *hyperparameters,
                                   size_t               *num_hyperparameters_ret) {
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	CCS_CHECK_ARY(num_hyperparameters, hyperparameters);
	if (!hyperparameters && !num_hyperparameters_ret)
		return -CCS_INVALID_VALUE;
	ccs_result_t err;
	UT_array *array;
	size_t count = 0;
	utarray_new(array, &_hyperparameter_icd);
	CCS_VALIDATE_ERR_GOTO(err, _get_hyperparameters(expression, array), errutarray);
	utarray_sort(array, &_hyper_sort);
	if (utarray_len(array) > 0) {
		ccs_hyperparameter_t  previous = NULL;
		ccs_hyperparameter_t *p_h = NULL;
		while ( (p_h = (ccs_hyperparameter_t *)utarray_next(array, p_h)) ) {
			if (*p_h != previous) {
				count += 1;
				previous = *p_h;
			}
		}
	} else
		count = 0;
	if (hyperparameters) {
		if (count > num_hyperparameters) {
			err = -CCS_INVALID_VALUE;
			goto errutarray;
		}
		ccs_hyperparameter_t  previous = NULL;
		ccs_hyperparameter_t *p_h = NULL;
		size_t index = 0;
		while ( (p_h = (ccs_hyperparameter_t *)utarray_next(array, p_h)) ) {
			if (*p_h != previous) {
				hyperparameters[index++] = *p_h;
				previous = *p_h;
			}
		}
		for (size_t i = count; i < num_hyperparameters; i++)
			hyperparameters[i] = NULL;
	}
	if (num_hyperparameters_ret)
		*num_hyperparameters_ret = count;
	err = CCS_SUCCESS;
errutarray:
	utarray_free(array);
	return err;
}

ccs_result_t
ccs_expression_check_context(ccs_expression_t expression,
                             ccs_context_t    context) {
	CCS_CHECK_OBJ(expression, CCS_EXPRESSION);
	ccs_result_t err;
	UT_array *array;
	utarray_new(array, &_hyperparameter_icd);
	CCS_VALIDATE_ERR_GOTO(err, _get_hyperparameters(expression, array), errutarray);
	utarray_sort(array, &_hyper_sort);
	if (utarray_len(array) > 0) {
		if (!context) {
			err = -CCS_INVALID_VALUE;
			goto errutarray;
		}
		ccs_hyperparameter_t  previous = NULL;
		ccs_hyperparameter_t *p_h = NULL;
		while ( (p_h = (ccs_hyperparameter_t *)utarray_next(array, p_h)) ) {
			if (*p_h != previous) {
				size_t index;
				CCS_VALIDATE_ERR_GOTO(err, ccs_context_get_hyperparameter_index(
					context, *p_h, &index), errutarray);
				previous = *p_h;
			}
		}
	}
	err = CCS_SUCCESS;
errutarray:
	utarray_free(array);
	return err;
}

