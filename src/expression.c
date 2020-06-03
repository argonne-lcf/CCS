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

#define eval_node(data, context, values, node, ht) do { \
	ccs_error_t err; \
	err = _ccs_expr_datum_eval(data->nodes, context, values, &node, ht); \
	if (err) \
		return err; \
} while(0)

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

static inline ccs_error_t
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

static ccs_error_t
_ccs_expr_not_equal_eval(_ccs_expression_data_t    *data,
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
		result->value.i = equal ? CCS_FALSE : CCS_TRUE;
	} else {
		if (err)
			return err;
		result->value.i = equal ? CCS_FALSE : CCS_TRUE;
	}
	result->type = CCS_BOOLEAN;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_not_equal_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_not_equal_eval
};

static ccs_error_t
_ccs_expr_less_eval(_ccs_expression_data_t *data,
                    ccs_configuration_space_t  context,
                    ccs_datum_t               *values,
                    ccs_datum_t               *result) {
	ccs_datum_t               left;
	ccs_datum_t               right;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	ccs_hyperparameter_type_t htr = CCS_HYPERPARAMETER_TYPE_MAX;

	eval_left_right(data, context, values, left, right, &htl, &htr);
	if (htl == CCS_CATEGORICAL || htr == CCS_CATEGORICAL)
		return -CCS_INVALID_VALUE;
	check_hypers(data->nodes[0], right, htl);
	check_hypers(data->nodes[1], left, htr);
	ccs_error_t err;
	if (htl == CCS_ORDINAL) {
		ccs_int_t cmp;
		err = ccs_ordinal_hyperparameter_compare_values(
			(ccs_hyperparameter_t)(data->nodes[0].value.o),
			left, right, &cmp);
		if (err)
			return err;
		result->type = CCS_BOOLEAN;
		result->value.i = (cmp < 0 ? CCS_TRUE : CCS_FALSE);
		return CCS_SUCCESS;
	}
	if (htr == CCS_ORDINAL) {
		ccs_int_t cmp;
		err = ccs_ordinal_hyperparameter_compare_values(
			(ccs_hyperparameter_t)(data->nodes[1].value.o),
			left, right, &cmp);
		if (err)
			return err;
		result->type = CCS_BOOLEAN;
		result->value.i = (cmp < 0 ? CCS_TRUE : CCS_FALSE);
		return CCS_SUCCESS;
	}
	ccs_int_t cmp;
	err = _ccs_datum_cmp_generic(&left, &right, &cmp);
	if (err)
		return err;
	result->type = CCS_BOOLEAN;
	result->value.i = cmp < 0 ? CCS_TRUE : CCS_FALSE;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_less_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_less_eval
};

static ccs_error_t
_ccs_expr_greater_eval(_ccs_expression_data_t *data,
                       ccs_configuration_space_t  context,
                       ccs_datum_t               *values,
                       ccs_datum_t               *result) {
	ccs_datum_t               left;
	ccs_datum_t               right;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	ccs_hyperparameter_type_t htr = CCS_HYPERPARAMETER_TYPE_MAX;

	eval_left_right(data, context, values, left, right, &htl, &htr);
	if (htl == CCS_CATEGORICAL || htr == CCS_CATEGORICAL)
		return -CCS_INVALID_VALUE;
	check_hypers(data->nodes[0], right, htl);
	check_hypers(data->nodes[1], left, htr);
	ccs_error_t err;
	if (htl == CCS_ORDINAL) {
		ccs_int_t cmp;
		err = ccs_ordinal_hyperparameter_compare_values(
			(ccs_hyperparameter_t)(data->nodes[0].value.o),
			left, right, &cmp);
		if (err)
			return err;
		result->type = CCS_BOOLEAN;
		result->value.i = (cmp > 0 ? CCS_TRUE : CCS_FALSE);
		return CCS_SUCCESS;
	}
	if (htr == CCS_ORDINAL) {
		ccs_int_t cmp;
		err = ccs_ordinal_hyperparameter_compare_values(
			(ccs_hyperparameter_t)(data->nodes[1].value.o),
			left, right, &cmp);
		if (err)
			return err;
		result->type = CCS_BOOLEAN;
		result->value.i = (cmp > 0 ? CCS_TRUE : CCS_FALSE);
		return CCS_SUCCESS;
	}
	ccs_int_t cmp;
	err = _ccs_datum_cmp_generic(&left, &right, &cmp);
	if (err)
		return err;
	result->type = CCS_BOOLEAN;
	result->value.i = cmp > 0 ? CCS_TRUE : CCS_FALSE;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_greater_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_greater_eval
};

static ccs_error_t
_ccs_expr_less_or_equal_eval(_ccs_expression_data_t *data,
                             ccs_configuration_space_t  context,
                             ccs_datum_t               *values,
                             ccs_datum_t               *result) {
	ccs_datum_t               left;
	ccs_datum_t               right;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	ccs_hyperparameter_type_t htr = CCS_HYPERPARAMETER_TYPE_MAX;

	eval_left_right(data, context, values, left, right, &htl, &htr);
	if (htl == CCS_CATEGORICAL || htr == CCS_CATEGORICAL)
		return -CCS_INVALID_VALUE;
	check_hypers(data->nodes[0], right, htl);
	check_hypers(data->nodes[1], left, htr);
	ccs_error_t err;
	if (htl == CCS_ORDINAL) {
		ccs_int_t cmp;
		err = ccs_ordinal_hyperparameter_compare_values(
			(ccs_hyperparameter_t)(data->nodes[0].value.o),
			left, right, &cmp);
		if (err)
			return err;
		result->type = CCS_BOOLEAN;
		result->value.i = (cmp <= 0 ? CCS_TRUE : CCS_FALSE);
		return CCS_SUCCESS;
	}
	if (htr == CCS_ORDINAL) {
		ccs_int_t cmp;
		err = ccs_ordinal_hyperparameter_compare_values(
			(ccs_hyperparameter_t)(data->nodes[1].value.o),
			left, right, &cmp);
		if (err)
			return err;
		result->type = CCS_BOOLEAN;
		result->value.i = (cmp <= 0 ? CCS_TRUE : CCS_FALSE);
		return CCS_SUCCESS;
	}
	ccs_int_t cmp;
	err = _ccs_datum_cmp_generic(&left, &right, &cmp);
	if (err)
		return err;
	result->type = CCS_BOOLEAN;
	result->value.i = cmp <= 0 ? CCS_TRUE : CCS_FALSE;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_less_or_equal_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_less_or_equal_eval
};

static ccs_error_t
_ccs_expr_greater_or_equal_eval(_ccs_expression_data_t *data,
                                ccs_configuration_space_t  context,
                                ccs_datum_t               *values,
                                ccs_datum_t               *result) {
	ccs_datum_t               left;
	ccs_datum_t               right;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	ccs_hyperparameter_type_t htr = CCS_HYPERPARAMETER_TYPE_MAX;

	eval_left_right(data, context, values, left, right, &htl, &htr);
	if (htl == CCS_CATEGORICAL || htr == CCS_CATEGORICAL)
		return -CCS_INVALID_VALUE;
	check_hypers(data->nodes[0], right, htl);
	check_hypers(data->nodes[1], left, htr);
	ccs_error_t err;
	if (htl == CCS_ORDINAL) {
		ccs_int_t cmp;
		err = ccs_ordinal_hyperparameter_compare_values(
			(ccs_hyperparameter_t)(data->nodes[0].value.o),
			left, right, &cmp);
		if (err)
			return err;
		result->type = CCS_BOOLEAN;
		result->value.i = (cmp >= 0 ? CCS_TRUE : CCS_FALSE);
		return CCS_SUCCESS;
	}
	if (htr == CCS_ORDINAL) {
		ccs_int_t cmp;
		err = ccs_ordinal_hyperparameter_compare_values(
			(ccs_hyperparameter_t)(data->nodes[1].value.o),
			left, right, &cmp);
		if (err)
			return err;
		result->type = CCS_BOOLEAN;
		result->value.i = (cmp >= 0 ? CCS_TRUE : CCS_FALSE);
		return CCS_SUCCESS;
	}
	ccs_int_t cmp;
	err = _ccs_datum_cmp_generic(&left, &right, &cmp);
	if (err)
		return err;
	result->type = CCS_BOOLEAN;
	result->value.i = cmp >= 0 ? CCS_TRUE : CCS_FALSE;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_greater_or_equal_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_greater_or_equal_eval
};

static ccs_error_t
_ccs_expr_in_eval(_ccs_expression_data_t *data,
                  ccs_configuration_space_t  context,
                  ccs_datum_t               *values,
                  ccs_datum_t               *result) {
	if (data->nodes[1].type != CCS_OBJECT)
		return -CCS_INVALID_VALUE;
	ccs_object_type_t type;
	ccs_error_t err;
	err = ccs_object_get_type(data->nodes[1].value.o, &type);
	if (err)
		return err;
	if (type != CCS_EXPRESSION)
		return -CCS_INVALID_VALUE;
	ccs_expression_type_t etype;
	err = ccs_expression_get_type((ccs_expression_t)(data->nodes[1].value.o), &etype);
	if (err)
		return err;
	if (etype != CCS_LIST)
		return -CCS_INVALID_VALUE;
	size_t num_nodes;
	err = ccs_expression_get_num_nodes((ccs_expression_t)(data->nodes[1].value.o), &num_nodes);
	if (err)
		return err;
	if (num_nodes == 0) {
		result->type = CCS_BOOLEAN;
		result->value.i = CCS_FALSE;
		return CCS_SUCCESS;
	}

	ccs_datum_t               left;
	ccs_hyperparameter_type_t htl = CCS_HYPERPARAMETER_TYPE_MAX;
	eval_node(data, context, values, left, &htl);
	for (size_t i = 0; i < num_nodes; i++) {
		ccs_datum_t right;
		err = ccs_expression_list_eval_node((ccs_expression_t)(data->nodes[1].value.o), context, values, i, &right);
		if (err)
			return err;
		check_hypers(data->nodes[0], right, htl);
		ccs_bool_t equal;
		ccs_error_t err = _ccs_datum_test_equal_generic(&left, &right, &equal);
		if (err) {
			if (err != -CCS_INVALID_VALUE)
				return err;
			else
				continue;
		}
		if (equal) {
			result->type = CCS_BOOLEAN;
			result->value.i = CCS_TRUE;
			return CCS_SUCCESS;
		}
	}
	result->type = CCS_BOOLEAN;
	result->value.i = CCS_FALSE;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_in_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_in_eval
};

static ccs_error_t
_ccs_expr_add_eval(_ccs_expression_data_t *data,
                   ccs_configuration_space_t  context,
                   ccs_datum_t               *values,
                   ccs_datum_t               *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			result->type = CCS_INTEGER;
			result->value.i = left.value.i + right.value.i;
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			result->type = CCS_FLOAT;
			result->value.f = left.value.i + right.value.f;
			return CCS_SUCCESS;
		}
		return -CCS_INVALID_VALUE;
	}
	if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			result->type = CCS_FLOAT;
			result->value.f = left.value.f + right.value.i;
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			result->type = CCS_FLOAT;
			result->value.f = left.value.f + right.value.f;
			return CCS_SUCCESS;
		}
	}
	return -CCS_INVALID_VALUE;
}

static _ccs_expression_ops_t _ccs_expr_add_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_add_eval
};

static ccs_error_t
_ccs_expr_substract_eval(_ccs_expression_data_t *data,
                         ccs_configuration_space_t  context,
                         ccs_datum_t               *values,
                         ccs_datum_t               *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			result->type = CCS_INTEGER;
			result->value.i = left.value.i - right.value.i;
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			result->type = CCS_FLOAT;
			result->value.f = left.value.i - right.value.f;
			return CCS_SUCCESS;
		}
		return -CCS_INVALID_VALUE;
	}
	if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			result->type = CCS_FLOAT;
			result->value.f = left.value.f - right.value.i;
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			result->type = CCS_FLOAT;
			result->value.f = left.value.f - right.value.f;
			return CCS_SUCCESS;
		}
	}
	return -CCS_INVALID_VALUE;
}

static _ccs_expression_ops_t _ccs_expr_substract_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_substract_eval
};

static ccs_error_t
_ccs_expr_multiply_eval(_ccs_expression_data_t    *data,
                        ccs_configuration_space_t  context,
                        ccs_datum_t               *values,
                        ccs_datum_t               *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			result->type = CCS_INTEGER;
			result->value.i = left.value.i * right.value.i;
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			result->type = CCS_FLOAT;
			result->value.f = left.value.i * right.value.f;
			return CCS_SUCCESS;
		}
		return -CCS_INVALID_VALUE;
	}
	if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			result->type = CCS_FLOAT;
			result->value.f = left.value.f * right.value.i;
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			result->type = CCS_FLOAT;
			result->value.f = left.value.f * right.value.f;
			return CCS_SUCCESS;
		}
	}
	return -CCS_INVALID_VALUE;
}

static _ccs_expression_ops_t _ccs_expr_multiply_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_multiply_eval
};

static ccs_error_t
_ccs_expr_divide_eval(_ccs_expression_data_t    *data,
                      ccs_configuration_space_t  context,
                      ccs_datum_t               *values,
                      ccs_datum_t               *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			if (right.value.i == 0)
				return -CCS_INVALID_VALUE;
			result->type = CCS_INTEGER;
			result->value.i = left.value.i / right.value.i;
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			if (right.value.f == 0.0)
				return -CCS_INVALID_VALUE;
			result->type = CCS_FLOAT;
			result->value.f = left.value.i / right.value.f;
			return CCS_SUCCESS;
		}
		return -CCS_INVALID_VALUE;
	}
	if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			if (right.value.i == 0)
				return -CCS_INVALID_VALUE;
			result->type = CCS_FLOAT;
			result->value.f = left.value.f / right.value.i;
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			if (right.value.f == 0.0)
				return -CCS_INVALID_VALUE;
			result->type = CCS_FLOAT;
			result->value.f = left.value.f / right.value.f;
			return CCS_SUCCESS;
		}
	}
	return -CCS_INVALID_VALUE;
}

static _ccs_expression_ops_t _ccs_expr_divide_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_divide_eval
};

static ccs_error_t
_ccs_expr_modulo_eval(_ccs_expression_data_t    *data,
                      ccs_configuration_space_t  context,
                      ccs_datum_t               *values,
                      ccs_datum_t               *result) {
	ccs_datum_t left;
	ccs_datum_t right;
	eval_left_right(data, context, values, left, right, NULL, NULL);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			if (right.value.i == 0)
				return -CCS_INVALID_VALUE;
			result->type = CCS_INTEGER;
			result->value.i = left.value.i % right.value.i;
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			if (right.value.f == 0.0)
				return -CCS_INVALID_VALUE;
			result->type = CCS_FLOAT;
			result->value.f = fmod(left.value.i, right.value.f);
			return CCS_SUCCESS;
		}
		return -CCS_INVALID_VALUE;
	}
	if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			if (right.value.i == 0)
				return -CCS_INVALID_VALUE;
			result->type = CCS_FLOAT;
			result->value.f = fmod(left.value.f, right.value.i);
			return CCS_SUCCESS;
		}
		if (right.type == CCS_FLOAT) {
			if (right.value.f == 0.0)
				return -CCS_INVALID_VALUE;
			result->type = CCS_FLOAT;
			result->value.f = fmod(left.value.f, right.value.f);
			return CCS_SUCCESS;
		}
	}
	return -CCS_INVALID_VALUE;
}

static _ccs_expression_ops_t _ccs_expr_modulo_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_modulo_eval
};

static ccs_error_t
_ccs_expr_positive_eval(_ccs_expression_data_t    *data,
                        ccs_configuration_space_t  context,
                        ccs_datum_t               *values,
                        ccs_datum_t               *result) {
	ccs_datum_t node;
	eval_node(data, context, values, node, NULL);
	if (node.type != CCS_INTEGER && node.type != CCS_FLOAT) {
		return -CCS_INVALID_VALUE;
	}
	*result = node;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_positive_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_positive_eval
};

static ccs_error_t
_ccs_expr_negative_eval(_ccs_expression_data_t    *data,
                        ccs_configuration_space_t  context,
                        ccs_datum_t               *values,
                        ccs_datum_t               *result) {
	ccs_datum_t node;
	eval_node(data, context, values, node, NULL);
	if (node.type == CCS_INTEGER) {
		result->type = node.type;
		result->value.i = - node.value.i;
	} else if (node.type == CCS_FLOAT) {
		result->type = node.type;
		result->value.f = - node.value.f;
	} else
		return -CCS_INVALID_VALUE;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_negative_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_negative_eval
};

static ccs_error_t
_ccs_expr_not_eval(_ccs_expression_data_t    *data,
                   ccs_configuration_space_t  context,
                   ccs_datum_t               *values,
                   ccs_datum_t               *result) {
	ccs_datum_t node;
	eval_node(data, context, values, node, NULL);
	if (node.type == CCS_BOOLEAN) {
		result->type = node.type;
		result->value.i = node.value.i ? CCS_FALSE : CCS_TRUE;
	} else
		return -CCS_INVALID_VALUE;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_not_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_not_eval
};

static ccs_error_t
_ccs_expr_list_eval(_ccs_expression_data_t    *data,
                   ccs_configuration_space_t  context,
                   ccs_datum_t               *values,
                   ccs_datum_t               *result) {
	return -CCS_UNSUPPORTED_OPERATION;
}

static _ccs_expression_ops_t _ccs_expr_list_ops = {
	{ &_ccs_expression_del },
	&_ccs_expr_list_eval
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
	case CCS_IN:
		return &_ccs_expr_in_ops;
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
	case CCS_LIST:
		return &_ccs_expr_list_ops;
		break;
	default:
		return NULL;
	}
}

ccs_error_t
ccs_create_expression(ccs_expression_type_t  type,
	              size_t                 num_nodes,
                      ccs_datum_t           *nodes,
                      ccs_expression_t      *expression_ret) {
	if (num_nodes && !nodes)
		return -CCS_INVALID_VALUE;
	if (!expression_ret)
		return -CCS_INVALID_VALUE;
	int arity = ccs_expression_arity[type];
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
	                 (_ccs_object_ops_t*)_ccs_expression_ops_broker(type));
	_ccs_expression_data_t *expression_data =
	    (_ccs_expression_data_t *)(mem + sizeof(struct _ccs_expression_s));
	expression_data->type = type;
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
ccs_create_binary_expression(ccs_expression_type_t  type,
                             ccs_datum_t            node_left,
                             ccs_datum_t            node_right,
                             ccs_expression_t      *expression_ret) {
	ccs_datum_t nodes[2];
	nodes[0] = node_left;
	nodes[1] = node_right;
	return ccs_create_expression(type, 2, nodes, expression_ret);
}

ccs_error_t
ccs_create_unary_expression(ccs_expression_type_t  type,
                            ccs_datum_t            node,
                            ccs_expression_t      *expression_ret) {
	return ccs_create_expression(type, 1, &node, expression_ret);
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

ccs_error_t
ccs_expression_get_num_nodes(ccs_expression_t  expression,
                             size_t           *num_nodes_ret) {
	if (!expression || !expression->data)
		return CCS_INVALID_OBJECT;
	if (!num_nodes_ret)
		return CCS_INVALID_VALUE;
	*num_nodes_ret = expression->data->num_nodes;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_expression_list_eval_node(ccs_expression_t           expression,
                              ccs_configuration_space_t  context,
                              ccs_datum_t               *values,
                              size_t                     index,
                              ccs_datum_t               *result) {
	if (!expression || !expression->data)
		return CCS_INVALID_OBJECT;
	if (!result)
		return CCS_INVALID_VALUE;
	ccs_error_t err;
	ccs_datum_t node;
	if (index >= expression->data->num_nodes)
		return -CCS_OUT_OF_BOUNDS;
	err = _ccs_expr_datum_eval(expression->data->nodes + index, context, values, &node, NULL);
	if (err)
		return err;
	*result = node;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_expression_get_type(ccs_expression_t       expression,
                        ccs_expression_type_t *type_ret) {
	if (!expression || !expression->data)
		return CCS_INVALID_OBJECT;
	*type_ret = expression->data->type;
	return CCS_SUCCESS;
}

#undef  utarray_oom
#define utarray_oom() { \
	return -CCS_ENOMEM; \
}

static ccs_error_t _get_hyperparameters(ccs_expression_t       expression,
                                        UT_array              *array) {
	if (!expression || !expression->data)
		return CCS_INVALID_OBJECT;
	ccs_error_t err;
	for (size_t i = 0; i < expression->data->num_nodes; i++) {
		ccs_datum_t *d = expression->data->nodes + i;
		if (d->type == CCS_OBJECT) {
			ccs_object_type_t t;
			err = ccs_object_get_type(d->value.o, &t);
			if (err)
				return err;
			if (t == CCS_HYPERPARAMETER)
				utarray_push_back(array, &(d->value.o));
			else if (t == CCS_EXPRESSION) {
				err = _get_hyperparameters((ccs_expression_t)(d->value.o), array);
				if (err)
					return err;
			}
		}
	}
	return CCS_SUCCESS;
}

static const UT_icd _hyperparameter_icd = {
	sizeof(ccs_hyperparameter_t),
	NULL,
	NULL,
	NULL,
};

static int hyper_sort(const void *a, const void *b) {
	ccs_hyperparameter_t ha = *(ccs_hyperparameter_t *)a;
	ccs_hyperparameter_t hb = *(ccs_hyperparameter_t *)b;
	return ha < hb ? -1 : ha > hb ? 1 : 0;
}

ccs_error_t
ccs_expression_get_hyperparameters(ccs_expression_t      expression,
                                   size_t                num_hyperparameters,
                                   ccs_hyperparameter_t *hyperparameters,
                                   size_t               *num_hyperparameters_ret) {
	if (num_hyperparameters && !hyperparameters)
		return -CCS_INVALID_VALUE;
	if (!hyperparameters && !num_hyperparameters_ret)
		return -CCS_INVALID_VALUE;
	ccs_error_t err;
	UT_array *array;
	utarray_new(array, &_hyperparameter_icd);
	err = _get_hyperparameters(expression, array);
	if (err) {
		utarray_free(array);
		return err;
	}
	utarray_sort(array, &hyper_sort);
	size_t count = 0;
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
			utarray_free(array);
			return -CCS_INVALID_VALUE;
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
	utarray_free(array);
	return CCS_SUCCESS;
}

ccs_error_t
ccs_expression_check_context(ccs_expression_t          expression,
                             ccs_configuration_space_t context) {
	ccs_error_t err;
	UT_array *array;
	utarray_new(array, &_hyperparameter_icd);
	err = _get_hyperparameters(expression, array);
	utarray_sort(array, &hyper_sort);
	if (err) {
		utarray_free(array);
		return err;
	}
	if (utarray_len(array) > 0) {
		if (!context) {
			utarray_free(array);
			return -CCS_INVALID_VALUE;
		}
		ccs_hyperparameter_t  previous = NULL;
		ccs_hyperparameter_t *p_h = NULL;
		while ( (p_h = (ccs_hyperparameter_t *)utarray_next(array, p_h)) ) {
			if (*p_h != previous) {
				size_t index;
				err = ccs_configuration_space_get_hyperparameter_index(
					context, *p_h, &index);
				if (err) {
					utarray_free(array);
					return err;
				}
				previous = *p_h;
			}
		}
	}
	utarray_free(array);
	return CCS_SUCCESS;
}

