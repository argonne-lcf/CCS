#include "cconfigspace_internal.h"
#include "expression_internal.h"
#include <math.h>
#include <string.h>
#include "utarray.h"

/* clang-format off */
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
	CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT,
	CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT,
	CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT, CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT,
	CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT, CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT, CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT, CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT,
	CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT, CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT,
	CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT, CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT, CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT,
	CCS_ASSOCIATIVITY_TYPE_RIGHT_TO_LEFT, CCS_ASSOCIATIVITY_TYPE_RIGHT_TO_LEFT, CCS_ASSOCIATIVITY_TYPE_RIGHT_TO_LEFT,
	CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT,
	CCS_ASSOCIATIVITY_TYPE_LEFT_TO_RIGHT,
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
/* clang-format on */

static inline _ccs_expression_ops_t *
ccs_expression_get_ops(ccs_expression_t expression)
{
	return (_ccs_expression_ops_t *)expression->obj.ops;
}

static ccs_error_t
_ccs_expression_del(ccs_object_t o)
{
	ccs_expression_t        d    = (ccs_expression_t)o;
	_ccs_expression_data_t *data = d->data;
	for (size_t i = 0; i < data->num_nodes; i++)
		ccs_release_object(data->nodes[i]);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_expression_data(
	_ccs_expression_data_t          *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	*cum_size += _ccs_serialize_bin_size_ccs_expression_type(data->type);
	*cum_size += _ccs_serialize_bin_size_size(data->num_nodes);
	for (size_t i = 0; i < data->num_nodes; i++)
		CCS_VALIDATE(data->nodes[i]->obj.ops->serialize_size(
			data->nodes[i], CCS_SERIALIZE_FORMAT_BINARY, cum_size,
			opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_expression_data(
	_ccs_expression_data_t          *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_type(
		data->type, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_serialize_bin_size(data->num_nodes, buffer_size, buffer));
	for (size_t i = 0; i < data->num_nodes; i++)
		CCS_VALIDATE(data->nodes[i]->obj.ops->serialize(
			data->nodes[i], CCS_SERIALIZE_FORMAT_BINARY,
			buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_expression(
	ccs_expression_t                 expression,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_expression_data_t *data =
		(_ccs_expression_data_t *)(expression->data);
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)expression);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_data(
		data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_expression(
	ccs_expression_t                 expression,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_expression_data_t *data =
		(_ccs_expression_data_t *)(expression->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)expression, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_data(
		data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_expression_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression(
			(ccs_expression_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_expression_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_expression(
			(ccs_expression_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_expr_node_eval(
	ccs_expression_t      n,
	ccs_context_t         context,
	ccs_datum_t          *values,
	ccs_datum_t          *result,
	ccs_parameter_type_t *ht)
{
	if (ht && n->data->type == CCS_EXPRESSION_TYPE_VARIABLE) {
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)n->data;
		CCS_VALIDATE(ccs_parameter_get_type(
			(ccs_parameter_t)(d->parameter), ht));
	}
	CCS_VALIDATE(ccs_expression_eval(n, context, values, result));
	return CCS_SUCCESS;
}

#define EVAL_NODE(data, context, values, node, ht)                             \
	do {                                                                   \
		CCS_VALIDATE(_ccs_expr_node_eval(                              \
			data->nodes[0], context, values, &node, ht));          \
	} while (0)

#define EVAL_LEFT_RIGHT(data, context, values, left, right, htl, htr)          \
	do {                                                                   \
		CCS_VALIDATE(_ccs_expr_node_eval(                              \
			data->nodes[0], context, values, &left, htl));         \
		CCS_VALIDATE(_ccs_expr_node_eval(                              \
			data->nodes[1], context, values, &right, htr));        \
	} while (0)

#define RETURN_IF_INACTIVE(node, result)                                       \
	do {                                                                   \
		if (node.type == CCS_INACTIVE) {                               \
			*result = ccs_inactive;                                \
			return CCS_SUCCESS;                                    \
		}                                                              \
	} while (0)

static ccs_error_t
_ccs_expr_or_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t left;
	ccs_datum_t right;
	// avoid inactive branch suppressing a parameter parameter
	// if the other branch is valid.
	CCS_VALIDATE(_ccs_expr_node_eval(
		data->nodes[0], context, values, &left, NULL));
	CCS_VALIDATE(_ccs_expr_node_eval(
		data->nodes[1], context, values, &right, NULL));
	CCS_REFUTE(
		left.type != CCS_BOOLEAN && left.type != CCS_INACTIVE,
		CCS_INVALID_VALUE);
	CCS_REFUTE(
		right.type != CCS_BOOLEAN && right.type != CCS_INACTIVE,
		CCS_INVALID_VALUE);
	if (left.type == CCS_BOOLEAN && left.value.i) {
		*result = ccs_true;
		return CCS_SUCCESS;
	}
	if (right.type == CCS_BOOLEAN && right.value.i) {
		*result = ccs_true;
		return CCS_SUCCESS;
	}
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	*result = ccs_false;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_or_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_or_eval};

static ccs_error_t
_ccs_expr_and_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t left;
	ccs_datum_t right;
	EVAL_LEFT_RIGHT(data, context, values, left, right, NULL, NULL);
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	CCS_REFUTE(
		left.type != CCS_BOOLEAN || right.type != CCS_BOOLEAN,
		CCS_INVALID_VALUE);
	*result = ((left.value.i && right.value.i) ? ccs_true : ccs_false);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_and_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_and_eval};

#define CHECK_VALUES(param, v)                                                 \
	do {                                                                   \
		ccs_bool_t valid;                                              \
		CCS_VALIDATE(ccs_parameter_check_value(param, v, &valid));     \
		CCS_REFUTE(!valid, CCS_INVALID_VALUE);                         \
	} while (0)

#define CHECK_PARAMETERS(param, v, t)                                           \
	do {                                                                    \
		if (t == CCS_PARAMETER_TYPE_ORDINAL ||                          \
		    t == CCS_PARAMETER_TYPE_CATEGORICAL) {                      \
			_ccs_expression_variable_data_t *d =                    \
				(_ccs_expression_variable_data_t *)param->data; \
			CHECK_VALUES(d->parameter, v);                          \
		} else if (                                                     \
			t == CCS_PARAMETER_TYPE_NUMERICAL ||                    \
			t == CCS_PARAMETER_TYPE_DISCRETE)                       \
			CCS_REFUTE(                                             \
				v.type != CCS_INTEGER && v.type != CCS_FLOAT,   \
				CCS_INVALID_VALUE);                             \
	} while (0)

static inline ccs_int_t
_ccs_string_cmp(const char *a, const char *b)
{
	if (a == b)
		return 0;
	if (!a)
		return -1;
	if (!b)
		return 1;
	return strcmp(a, b);
}

static inline ccs_bool_t
_ccs_datum_test_equal_generic(ccs_datum_t *a, ccs_datum_t *b, ccs_bool_t *equal)
{
	if (a->type == b->type) {
		switch (a->type) {
		case CCS_STRING:
			*equal = _ccs_string_cmp(a->value.s, b->value.s) == 0 ?
					 CCS_TRUE :
					 CCS_FALSE;
			break;
		case CCS_NONE:
			*equal = CCS_TRUE;
			break;
		default:
			*equal = memcmp(&(a->value), &(b->value),
					sizeof(ccs_value_t)) == 0 ?
					 CCS_TRUE :
					 CCS_FALSE;
		}
	} else {
		if (a->type == CCS_INTEGER && b->type == CCS_FLOAT) {
			*equal = (a->value.i == b->value.f) ? CCS_TRUE :
							      CCS_FALSE;
		} else if (a->type == CCS_FLOAT && b->type == CCS_INTEGER) {
			*equal = (a->value.f == b->value.i) ? CCS_TRUE :
							      CCS_FALSE;
		} else {
			*equal = CCS_FALSE;
			return CCS_FALSE;
		}
	}
	return CCS_TRUE;
}

static inline ccs_error_t
_ccs_datum_cmp_generic(ccs_datum_t *a, ccs_datum_t *b, ccs_int_t *cmp)
{
	if (a->type == b->type) {
		switch (a->type) {
		case CCS_STRING:
			*cmp = _ccs_string_cmp(a->value.s, b->value.s);
			break;
		case CCS_INTEGER:
			*cmp = a->value.i < b->value.i ? -1 :
			       a->value.i > b->value.i ? 1 :
							 0;
			break;
		case CCS_FLOAT:
			*cmp = a->value.f < b->value.f ? -1 :
			       a->value.f > b->value.f ? 1 :
							 0;
			break;
		default:
			CCS_RAISE(
				CCS_INVALID_VALUE, "Type %d is not comparable",
				a->type);
		}
	} else {
		if (a->type == CCS_INTEGER && b->type == CCS_FLOAT) {
			*cmp = a->value.i < b->value.f ? -1 :
			       a->value.i > b->value.f ? 1 :
							 0;
		} else if (a->type == CCS_FLOAT && b->type == CCS_INTEGER) {
			*cmp = a->value.f < b->value.i ? -1 :
			       a->value.f > b->value.i ? 1 :
							 0;
		} else {
			CCS_RAISE(
				CCS_INVALID_VALUE,
				"Types %d and %d are not comparable", a->type,
				b->type);
		}
	}
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_expr_equal_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t          left;
	ccs_datum_t          right;
	ccs_parameter_type_t htl = CCS_PARAMETER_TYPE_MAX;
	ccs_parameter_type_t htr = CCS_PARAMETER_TYPE_MAX;

	EVAL_LEFT_RIGHT(data, context, values, left, right, &htl, &htr);
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	CHECK_PARAMETERS(data->nodes[0], right, htl);
	CHECK_PARAMETERS(data->nodes[1], left, htr);
	ccs_bool_t equal;
	ccs_bool_t valid = _ccs_datum_test_equal_generic(&left, &right, &equal);
	if (htl == CCS_PARAMETER_TYPE_MAX && htr == CCS_PARAMETER_TYPE_MAX &&
	    !valid)
		CCS_RAISE(
			CCS_INVALID_VALUE, "Types %d and %d are not comparable",
			left.type, right.type);
	;
	*result = (equal ? ccs_true : ccs_false);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_equal_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_equal_eval};

static ccs_error_t
_ccs_expr_not_equal_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t          left;
	ccs_datum_t          right;
	ccs_parameter_type_t htl = CCS_PARAMETER_TYPE_MAX;
	ccs_parameter_type_t htr = CCS_PARAMETER_TYPE_MAX;

	EVAL_LEFT_RIGHT(data, context, values, left, right, &htl, &htr);
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	CHECK_PARAMETERS(data->nodes[0], right, htl);
	CHECK_PARAMETERS(data->nodes[1], left, htr);
	ccs_bool_t equal;
	ccs_bool_t valid = _ccs_datum_test_equal_generic(&left, &right, &equal);
	if (htl == CCS_PARAMETER_TYPE_MAX && htr == CCS_PARAMETER_TYPE_MAX &&
	    !valid)
		CCS_RAISE(
			CCS_INVALID_VALUE, "Types %d and %d are not comparable",
			left.type, right.type);
	;
	*result = (equal ? ccs_false : ccs_true);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_not_equal_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_not_equal_eval};

static ccs_error_t
_ccs_expr_less_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t          left;
	ccs_datum_t          right;
	ccs_parameter_type_t htl = CCS_PARAMETER_TYPE_MAX;
	ccs_parameter_type_t htr = CCS_PARAMETER_TYPE_MAX;

	EVAL_LEFT_RIGHT(data, context, values, left, right, &htl, &htr);
	CCS_REFUTE(
		htl == CCS_PARAMETER_TYPE_CATEGORICAL ||
			htr == CCS_PARAMETER_TYPE_CATEGORICAL,
		CCS_INVALID_VALUE);
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	CHECK_PARAMETERS(data->nodes[0], right, htl);
	CHECK_PARAMETERS(data->nodes[1], left, htr);
	if (htl == CCS_PARAMETER_TYPE_ORDINAL) {
		ccs_int_t                        cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[0]->data;
		CCS_VALIDATE(ccs_ordinal_parameter_compare_values(
			d->parameter, left, right, &cmp));
		*result = (cmp < 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	if (htr == CCS_PARAMETER_TYPE_ORDINAL) {
		ccs_int_t                        cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[1]->data;
		CCS_VALIDATE(ccs_ordinal_parameter_compare_values(
			d->parameter, left, right, &cmp));
		*result = (cmp < 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	ccs_int_t cmp;
	CCS_VALIDATE(_ccs_datum_cmp_generic(&left, &right, &cmp));
	*result = (cmp < 0 ? ccs_true : ccs_false);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_less_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_less_eval};

static ccs_error_t
_ccs_expr_greater_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t          left;
	ccs_datum_t          right;
	ccs_parameter_type_t htl = CCS_PARAMETER_TYPE_MAX;
	ccs_parameter_type_t htr = CCS_PARAMETER_TYPE_MAX;

	EVAL_LEFT_RIGHT(data, context, values, left, right, &htl, &htr);
	CCS_REFUTE(
		htl == CCS_PARAMETER_TYPE_CATEGORICAL ||
			htr == CCS_PARAMETER_TYPE_CATEGORICAL,
		CCS_INVALID_VALUE);
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	CHECK_PARAMETERS(data->nodes[0], right, htl);
	CHECK_PARAMETERS(data->nodes[1], left, htr);
	if (htl == CCS_PARAMETER_TYPE_ORDINAL) {
		ccs_int_t                        cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[0]->data;
		CCS_VALIDATE(ccs_ordinal_parameter_compare_values(
			d->parameter, left, right, &cmp));
		*result = (cmp > 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	if (htr == CCS_PARAMETER_TYPE_ORDINAL) {
		ccs_int_t                        cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[1]->data;
		CCS_VALIDATE(ccs_ordinal_parameter_compare_values(
			d->parameter, left, right, &cmp));
		*result = (cmp > 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	ccs_int_t cmp;
	CCS_VALIDATE(_ccs_datum_cmp_generic(&left, &right, &cmp));
	*result = (cmp > 0 ? ccs_true : ccs_false);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_greater_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_greater_eval};

static ccs_error_t
_ccs_expr_less_or_equal_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t          left;
	ccs_datum_t          right;
	ccs_parameter_type_t htl = CCS_PARAMETER_TYPE_MAX;
	ccs_parameter_type_t htr = CCS_PARAMETER_TYPE_MAX;

	EVAL_LEFT_RIGHT(data, context, values, left, right, &htl, &htr);
	CCS_REFUTE(
		htl == CCS_PARAMETER_TYPE_CATEGORICAL ||
			htr == CCS_PARAMETER_TYPE_CATEGORICAL,
		CCS_INVALID_VALUE);
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	CHECK_PARAMETERS(data->nodes[0], right, htl);
	CHECK_PARAMETERS(data->nodes[1], left, htr);
	if (htl == CCS_PARAMETER_TYPE_ORDINAL) {
		ccs_int_t                        cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[0]->data;
		CCS_VALIDATE(ccs_ordinal_parameter_compare_values(
			d->parameter, left, right, &cmp));
		*result = (cmp <= 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	if (htr == CCS_PARAMETER_TYPE_ORDINAL) {
		ccs_int_t                        cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[1]->data;
		CCS_VALIDATE(ccs_ordinal_parameter_compare_values(
			d->parameter, left, right, &cmp));
		*result = (cmp <= 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	ccs_int_t cmp;
	CCS_VALIDATE(_ccs_datum_cmp_generic(&left, &right, &cmp));
	*result = (cmp <= 0 ? ccs_true : ccs_false);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_less_or_equal_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_less_or_equal_eval};

static ccs_error_t
_ccs_expr_greater_or_equal_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t          left;
	ccs_datum_t          right;
	ccs_parameter_type_t htl = CCS_PARAMETER_TYPE_MAX;
	ccs_parameter_type_t htr = CCS_PARAMETER_TYPE_MAX;

	EVAL_LEFT_RIGHT(data, context, values, left, right, &htl, &htr);
	CCS_REFUTE(
		htl == CCS_PARAMETER_TYPE_CATEGORICAL ||
			htr == CCS_PARAMETER_TYPE_CATEGORICAL,
		CCS_INVALID_VALUE);
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	CHECK_PARAMETERS(data->nodes[0], right, htl);
	CHECK_PARAMETERS(data->nodes[1], left, htr);
	if (htl == CCS_PARAMETER_TYPE_ORDINAL) {
		ccs_int_t                        cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[0]->data;
		CCS_VALIDATE(ccs_ordinal_parameter_compare_values(
			d->parameter, left, right, &cmp));
		*result = (cmp >= 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	if (htr == CCS_PARAMETER_TYPE_ORDINAL) {
		ccs_int_t                        cmp;
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)data->nodes[1]->data;
		CCS_VALIDATE(ccs_ordinal_parameter_compare_values(
			d->parameter, left, right, &cmp));
		*result = (cmp >= 0 ? ccs_true : ccs_false);
		return CCS_SUCCESS;
	}
	ccs_int_t cmp;
	CCS_VALIDATE(_ccs_datum_cmp_generic(&left, &right, &cmp));
	*result = (cmp >= 0 ? ccs_true : ccs_false);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_greater_or_equal_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_greater_or_equal_eval};

static ccs_error_t
_ccs_expr_in_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_expression_type_t etype;
	CCS_VALIDATE(ccs_expression_get_type(data->nodes[1], &etype));
	CCS_REFUTE(etype != CCS_EXPRESSION_TYPE_LIST, CCS_INVALID_VALUE);
	size_t               num_nodes;
	ccs_datum_t          left;
	ccs_bool_t           inactive = CCS_FALSE;
	ccs_parameter_type_t htl      = CCS_PARAMETER_TYPE_MAX;
	EVAL_NODE(data, context, values, left, &htl);
	RETURN_IF_INACTIVE(left, result);
	CCS_VALIDATE(ccs_expression_get_num_nodes(data->nodes[1], &num_nodes));
	for (size_t i = 0; i < num_nodes; i++) {
		ccs_datum_t right;
		CCS_VALIDATE(ccs_expression_list_eval_node(
			data->nodes[1], context, values, i, &right));
		if (right.type == CCS_INACTIVE)
			inactive = CCS_TRUE;
		CHECK_PARAMETERS(data->nodes[0], right, htl);
		ccs_bool_t equal;
		_ccs_datum_test_equal_generic(&left, &right, &equal);
		if (equal) {
			*result = ccs_true;
			return CCS_SUCCESS;
		}
	}
	if (inactive)
		*result = ccs_inactive;
	else
		*result = ccs_false;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_in_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_in_eval};

static ccs_error_t
_ccs_expr_add_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t left;
	ccs_datum_t right;
	EVAL_LEFT_RIGHT(data, context, values, left, right, NULL, NULL);
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			*result = ccs_int(left.value.i + right.value.i);
			return CCS_SUCCESS;
		} else if (right.type == CCS_FLOAT) {
			*result = ccs_float(left.value.i + right.value.f);
			return CCS_SUCCESS;
		}
	} else if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			*result = ccs_float(left.value.f + right.value.i);
			return CCS_SUCCESS;
		} else if (right.type == CCS_FLOAT) {
			*result = ccs_float(left.value.f + right.value.f);
			return CCS_SUCCESS;
		}
	}
	CCS_RAISE(CCS_INVALID_VALUE, "Incompatibles type for addition");
}

static _ccs_expression_ops_t _ccs_expr_add_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_add_eval};

static ccs_error_t
_ccs_expr_substract_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t left;
	ccs_datum_t right;
	EVAL_LEFT_RIGHT(data, context, values, left, right, NULL, NULL);
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			*result = ccs_int(left.value.i - right.value.i);
			return CCS_SUCCESS;
		} else if (right.type == CCS_FLOAT) {
			*result = ccs_float(left.value.i - right.value.f);
			return CCS_SUCCESS;
		}
	} else if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			*result = ccs_float(left.value.f - right.value.i);
			return CCS_SUCCESS;
		} else if (right.type == CCS_FLOAT) {
			*result = ccs_float(left.value.f - right.value.f);
			return CCS_SUCCESS;
		}
	}
	CCS_RAISE(CCS_INVALID_VALUE, "Incompatible types for substraction");
}

static _ccs_expression_ops_t _ccs_expr_substract_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_substract_eval};

static ccs_error_t
_ccs_expr_multiply_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t left;
	ccs_datum_t right;
	EVAL_LEFT_RIGHT(data, context, values, left, right, NULL, NULL);
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			*result = ccs_int(left.value.i * right.value.i);
			return CCS_SUCCESS;
		} else if (right.type == CCS_FLOAT) {
			*result = ccs_float(left.value.i * right.value.f);
			return CCS_SUCCESS;
		}
	} else if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			*result = ccs_float(left.value.f * right.value.i);
			return CCS_SUCCESS;
		} else if (right.type == CCS_FLOAT) {
			*result = ccs_float(left.value.f * right.value.f);
			return CCS_SUCCESS;
		}
	}
	CCS_RAISE(CCS_INVALID_VALUE, "Incompatible types for multiplication");
}

static _ccs_expression_ops_t _ccs_expr_multiply_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_multiply_eval};

static ccs_error_t
_ccs_expr_divide_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t left;
	ccs_datum_t right;
	EVAL_LEFT_RIGHT(data, context, values, left, right, NULL, NULL);
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			CCS_REFUTE(right.value.i == 0, CCS_INVALID_VALUE);
			*result = ccs_int(left.value.i / right.value.i);
			return CCS_SUCCESS;
		} else if (right.type == CCS_FLOAT) {
			CCS_REFUTE(right.value.f == 0.0, CCS_INVALID_VALUE);
			*result = ccs_float(left.value.i / right.value.f);
			return CCS_SUCCESS;
		}
	} else if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			CCS_REFUTE(right.value.i == 0, CCS_INVALID_VALUE);
			*result = ccs_float(left.value.f / right.value.i);
			return CCS_SUCCESS;
		} else if (right.type == CCS_FLOAT) {
			CCS_REFUTE(right.value.f == 0.0, CCS_INVALID_VALUE);
			*result = ccs_float(left.value.f / right.value.f);
			return CCS_SUCCESS;
		}
	}
	CCS_RAISE(CCS_INVALID_VALUE, "Incompatible types for division");
}

static _ccs_expression_ops_t _ccs_expr_divide_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_divide_eval};

static ccs_error_t
_ccs_expr_modulo_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t left;
	ccs_datum_t right;
	EVAL_LEFT_RIGHT(data, context, values, left, right, NULL, NULL);
	RETURN_IF_INACTIVE(left, result);
	RETURN_IF_INACTIVE(right, result);
	if (left.type == CCS_INTEGER) {
		if (right.type == CCS_INTEGER) {
			CCS_REFUTE(right.value.i == 0, CCS_INVALID_VALUE);
			*result = ccs_int(left.value.i % right.value.i);
			return CCS_SUCCESS;
		} else if (right.type == CCS_FLOAT) {
			CCS_REFUTE(right.value.f == 0.0, CCS_INVALID_VALUE);
			*result = ccs_float(fmod(left.value.i, right.value.f));
			return CCS_SUCCESS;
		}
	} else if (left.type == CCS_FLOAT) {
		if (right.type == CCS_INTEGER) {
			CCS_REFUTE(right.value.i == 0, CCS_INVALID_VALUE);
			*result = ccs_float(fmod(left.value.f, right.value.i));
			return CCS_SUCCESS;
		} else if (right.type == CCS_FLOAT) {
			CCS_REFUTE(right.value.f == 0.0, CCS_INVALID_VALUE);
			*result = ccs_float(fmod(left.value.f, right.value.f));
			return CCS_SUCCESS;
		}
	}
	CCS_RAISE(CCS_INVALID_VALUE, "Incompatible types for modulo");
}

static _ccs_expression_ops_t _ccs_expr_modulo_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_modulo_eval};

static ccs_error_t
_ccs_expr_positive_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t node;
	EVAL_NODE(data, context, values, node, NULL);
	RETURN_IF_INACTIVE(node, result);
	CCS_REFUTE(
		node.type != CCS_INTEGER && node.type != CCS_FLOAT,
		CCS_INVALID_VALUE);
	*result = node;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_positive_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_positive_eval};

static ccs_error_t
_ccs_expr_negative_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t node;
	EVAL_NODE(data, context, values, node, NULL);
	RETURN_IF_INACTIVE(node, result);
	CCS_REFUTE(
		node.type != CCS_INTEGER && node.type != CCS_FLOAT,
		CCS_INVALID_VALUE);
	if (node.type == CCS_INTEGER) {
		*result = ccs_int(-node.value.i);
	} else if (node.type == CCS_FLOAT) {
		*result = ccs_float(-node.value.f);
	}
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_negative_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_negative_eval};

static ccs_error_t
_ccs_expr_not_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	ccs_datum_t node;
	EVAL_NODE(data, context, values, node, NULL);
	RETURN_IF_INACTIVE(node, result);
	CCS_REFUTE(node.type != CCS_BOOLEAN, CCS_INVALID_VALUE);
	*result = (node.value.i ? ccs_false : ccs_true);
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_not_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_not_eval};

static ccs_error_t
_ccs_expr_list_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	(void)data;
	(void)context;
	(void)values;
	(void)result;
	CCS_RAISE(CCS_UNSUPPORTED_OPERATION, "Lists cannot be avaluated");
}

static _ccs_expression_ops_t _ccs_expr_list_ops = {
	{&_ccs_expression_del, &_ccs_expression_serialize_size,
	 &_ccs_expression_serialize},
	&_ccs_expr_list_eval};

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_expression_literal_data(
	_ccs_expression_literal_data_t  *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_data(
		&data->expr, cum_size, opts));
	*cum_size += _ccs_serialize_bin_size_ccs_datum(data->value);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_expression_literal_data(
	_ccs_expression_literal_data_t  *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_data(
		&data->expr, buffer_size, buffer, opts));
	CCS_VALIDATE(
		_ccs_serialize_bin_ccs_datum(data->value, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_expression_literal(
	ccs_expression_t                 expression,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_expression_literal_data_t *data =
		(_ccs_expression_literal_data_t *)(expression->data);
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)expression);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_literal_data(
		data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_expression_literal(
	ccs_expression_t                 expression,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_expression_literal_data_t *data =
		(_ccs_expression_literal_data_t *)(expression->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)expression, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_literal_data(
		data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_expression_literal_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_literal(
			(ccs_expression_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_expression_literal_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_literal(
			(ccs_expression_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_expr_literal_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	(void)context;
	(void)values;
	_ccs_expression_literal_data_t *d =
		(_ccs_expression_literal_data_t *)data;
	*result = d->value;
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_literal_ops = {
	{&_ccs_expression_del, &_ccs_expression_literal_serialize_size,
	 &_ccs_expression_literal_serialize},
	&_ccs_expr_literal_eval};

static ccs_error_t
_ccs_expr_variable_del(ccs_object_t o)
{
	_ccs_expression_variable_data_t *d =
		(_ccs_expression_variable_data_t *)((ccs_expression_t)o)->data;
	CCS_VALIDATE(ccs_release_object(d->parameter));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_expression_variable_data(
	_ccs_expression_variable_data_t *data,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_data(
		&data->expr, cum_size, opts));
	*cum_size += _ccs_serialize_bin_size_ccs_object(data->parameter);
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_expression_variable_data(
	_ccs_expression_variable_data_t *data,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_data(
		&data->expr, buffer_size, buffer, opts));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object(
		data->parameter, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_size_ccs_expression_variable(
	ccs_expression_t                 expression,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_expression_variable_data_t *data =
		(_ccs_expression_variable_data_t *)(expression->data);
	*cum_size += _ccs_serialize_bin_size_ccs_object_internal(
		(_ccs_object_internal_t *)expression);
	CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_variable_data(
		data, cum_size, opts));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_serialize_bin_ccs_expression_variable(
	ccs_expression_t                 expression,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	_ccs_expression_variable_data_t *data =
		(_ccs_expression_variable_data_t *)(expression->data);
	CCS_VALIDATE(_ccs_serialize_bin_ccs_object_internal(
		(_ccs_object_internal_t *)expression, buffer_size, buffer));
	CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_variable_data(
		data, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_expression_variable_serialize_size(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *cum_size,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_size_ccs_expression_variable(
			(ccs_expression_t)object, cum_size, opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data_size(
		object, format, cum_size, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_expression_variable_serialize(
	ccs_object_t                     object,
	ccs_serialize_format_t           format,
	size_t                          *buffer_size,
	char                           **buffer,
	_ccs_object_serialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_serialize_bin_ccs_expression_variable(
			(ccs_expression_t)object, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_serialize_user_data(
		object, format, buffer_size, buffer, opts));
	return CCS_SUCCESS;
}

static ccs_error_t
_ccs_expr_variable_eval(
	_ccs_expression_data_t *data,
	ccs_context_t           context,
	ccs_datum_t            *values,
	ccs_datum_t            *result)
{
	_ccs_expression_variable_data_t *d =
		(_ccs_expression_variable_data_t *)data;
	size_t index;
	CCS_CHECK_PTR(values);
	CCS_VALIDATE(ccs_context_get_parameter_index(
		context, (ccs_parameter_t)(d->parameter), &index));
	*result = values[index];
	return CCS_SUCCESS;
}

static _ccs_expression_ops_t _ccs_expr_variable_ops = {
	{&_ccs_expr_variable_del, &_ccs_expression_variable_serialize_size,
	 &_ccs_expression_variable_serialize},
	&_ccs_expr_variable_eval};

static inline _ccs_expression_ops_t *
_ccs_expression_ops_broker(ccs_expression_type_t expression_type)
{
	switch (expression_type) {
	case CCS_EXPRESSION_TYPE_OR:
		return &_ccs_expr_or_ops;
		break;
	case CCS_EXPRESSION_TYPE_AND:
		return &_ccs_expr_and_ops;
		break;
	case CCS_EXPRESSION_TYPE_EQUAL:
		return &_ccs_expr_equal_ops;
		break;
	case CCS_EXPRESSION_TYPE_NOT_EQUAL:
		return &_ccs_expr_not_equal_ops;
		break;
	case CCS_EXPRESSION_TYPE_LESS:
		return &_ccs_expr_less_ops;
		break;
	case CCS_EXPRESSION_TYPE_GREATER:
		return &_ccs_expr_greater_ops;
		break;
	case CCS_EXPRESSION_TYPE_LESS_OR_EQUAL:
		return &_ccs_expr_less_or_equal_ops;
		break;
	case CCS_EXPRESSION_TYPE_GREATER_OR_EQUAL:
		return &_ccs_expr_greater_or_equal_ops;
		break;
	case CCS_EXPRESSION_TYPE_ADD:
		return &_ccs_expr_add_ops;
		break;
	case CCS_EXPRESSION_TYPE_SUBSTRACT:
		return &_ccs_expr_substract_ops;
		break;
	case CCS_EXPRESSION_TYPE_MULTIPLY:
		return &_ccs_expr_multiply_ops;
		break;
	case CCS_EXPRESSION_TYPE_DIVIDE:
		return &_ccs_expr_divide_ops;
		break;
	case CCS_EXPRESSION_TYPE_MODULO:
		return &_ccs_expr_modulo_ops;
		break;
	case CCS_EXPRESSION_TYPE_POSITIVE:
		return &_ccs_expr_positive_ops;
		break;
	case CCS_EXPRESSION_TYPE_NEGATIVE:
		return &_ccs_expr_negative_ops;
		break;
	case CCS_EXPRESSION_TYPE_NOT:
		return &_ccs_expr_not_ops;
		break;
	case CCS_EXPRESSION_TYPE_IN:
		return &_ccs_expr_in_ops;
		break;
	case CCS_EXPRESSION_TYPE_LIST:
		return &_ccs_expr_list_ops;
		break;
	case CCS_EXPRESSION_TYPE_LITERAL:
		return &_ccs_expr_literal_ops;
		break;
	case CCS_EXPRESSION_TYPE_VARIABLE:
		return &_ccs_expr_variable_ops;
		break;
	default:
		return NULL;
	}
}

ccs_error_t
ccs_create_literal(ccs_datum_t value, ccs_expression_t *expression_ret)
{
	CCS_REFUTE(
		value.type < CCS_NONE || value.type > CCS_STRING,
		CCS_INVALID_VALUE);
	CCS_CHECK_PTR(expression_ret);
	size_t size_str = 0;
	if (value.type == CCS_STRING && value.value.s) {
		size_str = strlen(value.value.s) + 1;
	}
	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_expression_s) +
			   sizeof(struct _ccs_expression_literal_data_s) +
			   size_str);
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	ccs_expression_t expression = (ccs_expression_t)mem;
	_ccs_object_init(
		&(expression->obj), CCS_OBJECT_TYPE_EXPRESSION,
		(_ccs_object_ops_t *)_ccs_expression_ops_broker(
			CCS_EXPRESSION_TYPE_LITERAL));
	_ccs_expression_literal_data_t *expression_data =
		(_ccs_expression_literal_data_t
			 *)(mem + sizeof(struct _ccs_expression_s));
	expression_data->expr.type      = CCS_EXPRESSION_TYPE_LITERAL;
	expression_data->expr.num_nodes = 0;
	expression_data->expr.nodes     = NULL;
	if (size_str) {
		char *str_pool =
			(char *)(mem + sizeof(struct _ccs_expression_s) + sizeof(struct _ccs_expression_literal_data_s));
		expression_data->value = ccs_string(str_pool);
		strcpy(str_pool, value.value.s);
	} else {
		expression_data->value       = value;
		expression_data->value.flags = CCS_FLAG_DEFAULT;
	}
	expression->data = (_ccs_expression_data_t *)expression_data;
	*expression_ret  = expression;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_create_variable(ccs_parameter_t parameter, ccs_expression_t *expression_ret)
{
	CCS_CHECK_OBJ(parameter, CCS_OBJECT_TYPE_PARAMETER);
	CCS_CHECK_PTR(expression_ret);
	ccs_error_t err;
	uintptr_t   mem = (uintptr_t)calloc(
                1, sizeof(struct _ccs_expression_s) +
                           sizeof(struct _ccs_expression_variable_data_s));
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);
	CCS_VALIDATE_ERR_GOTO(err, ccs_retain_object(parameter), errmem);
	ccs_expression_t expression;
	expression = (ccs_expression_t)mem;
	_ccs_object_init(
		&(expression->obj), CCS_OBJECT_TYPE_EXPRESSION,
		(_ccs_object_ops_t *)_ccs_expression_ops_broker(
			CCS_EXPRESSION_TYPE_VARIABLE));
	_ccs_expression_variable_data_t *expression_data;
	expression_data                 = (_ccs_expression_variable_data_t
                                   *)(mem + sizeof(struct _ccs_expression_s));
	expression_data->expr.type      = CCS_EXPRESSION_TYPE_VARIABLE;
	expression_data->expr.num_nodes = 0;
	expression_data->expr.nodes     = NULL;
	expression_data->parameter      = parameter;
	expression->data = (_ccs_expression_data_t *)expression_data;
	*expression_ret  = expression;
	return CCS_SUCCESS;
errmem:
	free((void *)mem);
	return err;
}

ccs_error_t
ccs_create_expression(
	ccs_expression_type_t type,
	size_t                num_nodes,
	ccs_datum_t          *nodes,
	ccs_expression_t     *expression_ret)
{
	CCS_CHECK_ARY(num_nodes, nodes);
	CCS_CHECK_PTR(expression_ret);
	CCS_REFUTE(
		type < CCS_EXPRESSION_TYPE_OR ||
			type > CCS_EXPRESSION_TYPE_LIST,
		CCS_INVALID_VALUE);
	int arity = ccs_expression_arity[type];
	CCS_REFUTE(arity >= 0 && num_nodes != (size_t)arity, CCS_INVALID_VALUE);
	ccs_error_t err;
	for (size_t i = 0; i < num_nodes; i++) {
		if (nodes[i].type == CCS_OBJECT) {
			ccs_object_type_t object_type;
			CCS_VALIDATE(ccs_object_get_type(
				nodes[i].value.o, &object_type));
			CCS_REFUTE(
				object_type != CCS_OBJECT_TYPE_PARAMETER &&
					object_type !=
						CCS_OBJECT_TYPE_EXPRESSION,
				CCS_INVALID_VALUE);
		} else
			CCS_REFUTE(
				nodes[i].type < CCS_NONE ||
					nodes[i].type > CCS_STRING,
				CCS_INVALID_VALUE);
	}

	uintptr_t mem = (uintptr_t)calloc(
		1, sizeof(struct _ccs_expression_s) +
			   sizeof(struct _ccs_expression_data_s) +
			   num_nodes * sizeof(ccs_expression_t));
	CCS_REFUTE(!mem, CCS_OUT_OF_MEMORY);

	ccs_expression_t expression = (ccs_expression_t)mem;
	_ccs_object_init(
		&(expression->obj), CCS_OBJECT_TYPE_EXPRESSION,
		(_ccs_object_ops_t *)_ccs_expression_ops_broker(type));
	_ccs_expression_data_t *expression_data =
		(_ccs_expression_data_t
			 *)(mem + sizeof(struct _ccs_expression_s));
	expression_data->type      = type;
	expression_data->num_nodes = num_nodes;
	expression_data->nodes =
		(ccs_expression_t
			 *)(mem + sizeof(struct _ccs_expression_s) + sizeof(struct _ccs_expression_data_s));
	for (size_t i = 0; i < num_nodes; i++) {
		if (nodes[i].type == CCS_OBJECT) {
			ccs_object_type_t t;
			CCS_VALIDATE_ERR_GOTO(
				err, ccs_object_get_type(nodes[i].value.o, &t),
				cleanup);
			if (t == CCS_OBJECT_TYPE_EXPRESSION) {
				CCS_VALIDATE_ERR_GOTO(
					err,
					ccs_retain_object(nodes[i].value.o),
					cleanup);
				expression_data->nodes[i] =
					(ccs_expression_t)nodes[i].value.o;
			} else {
				CCS_VALIDATE_ERR_GOTO(
					err,
					ccs_create_variable(
						(ccs_parameter_t)nodes[i]
							.value.o,
						expression_data->nodes + i),
					cleanup);
			}
		} else {
			CCS_VALIDATE_ERR_GOTO(
				err,
				ccs_create_literal(
					nodes[i], expression_data->nodes + i),
				cleanup);
		}
	}
	expression->data = expression_data;
	*expression_ret  = expression;
	return CCS_SUCCESS;
cleanup:
	for (size_t i = 0; i < num_nodes; i++) {
		if (expression_data->nodes[i])
			ccs_release_object(expression_data->nodes[i]);
	}
	free((void *)mem);
	return err;
}

ccs_error_t
ccs_create_binary_expression(
	ccs_expression_type_t type,
	ccs_datum_t           node_left,
	ccs_datum_t           node_right,
	ccs_expression_t     *expression_ret)
{
	ccs_datum_t nodes[2];
	nodes[0] = node_left;
	nodes[1] = node_right;
	CCS_VALIDATE(ccs_create_expression(type, 2, nodes, expression_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_create_unary_expression(
	ccs_expression_type_t type,
	ccs_datum_t           node,
	ccs_expression_t     *expression_ret)
{
	CCS_VALIDATE(ccs_create_expression(type, 1, &node, expression_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_expression_eval(
	ccs_expression_t expression,
	ccs_context_t    context,
	ccs_datum_t     *values,
	ccs_datum_t     *result_ret)
{
	CCS_CHECK_OBJ(expression, CCS_OBJECT_TYPE_EXPRESSION);
	CCS_CHECK_PTR(result_ret);
	_ccs_expression_ops_t *ops = ccs_expression_get_ops(expression);
	CCS_VALIDATE(ops->eval(expression->data, context, values, result_ret));
	return CCS_SUCCESS;
}

ccs_error_t
ccs_expression_get_num_nodes(ccs_expression_t expression, size_t *num_nodes_ret)
{
	CCS_CHECK_OBJ(expression, CCS_OBJECT_TYPE_EXPRESSION);
	CCS_CHECK_PTR(num_nodes_ret);
	*num_nodes_ret = expression->data->num_nodes;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_expression_get_nodes(
	ccs_expression_t  expression,
	size_t            num_nodes,
	ccs_expression_t *nodes,
	size_t           *num_nodes_ret)
{
	CCS_CHECK_OBJ(expression, CCS_OBJECT_TYPE_EXPRESSION);
	CCS_CHECK_ARY(num_nodes, nodes);
	CCS_REFUTE(!num_nodes_ret && !nodes, CCS_INVALID_VALUE);
	size_t count = expression->data->num_nodes;
	if (nodes) {
		CCS_REFUTE(num_nodes < count, CCS_INVALID_VALUE);
		ccs_expression_t *p_nodes = expression->data->nodes;
		for (size_t i = 0; i < count; i++)
			nodes[i] = p_nodes[i];
		for (size_t i = count; i < num_nodes; i++)
			nodes[i] = NULL;
	}
	if (num_nodes_ret)
		*num_nodes_ret = count;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_expression_list_eval_node(
	ccs_expression_t expression,
	ccs_context_t    context,
	ccs_datum_t     *values,
	size_t           index,
	ccs_datum_t     *result)
{
	CCS_CHECK_OBJ(expression, CCS_OBJECT_TYPE_EXPRESSION);
	CCS_CHECK_PTR(result);
	CCS_REFUTE(
		expression->data->type != CCS_EXPRESSION_TYPE_LIST,
		CCS_INVALID_EXPRESSION);
	ccs_datum_t node;
	CCS_REFUTE(index >= expression->data->num_nodes, CCS_OUT_OF_BOUNDS);
	CCS_VALIDATE(_ccs_expr_node_eval(
		expression->data->nodes[index], context, values, &node, NULL));
	*result = node;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_expression_get_type(
	ccs_expression_t       expression,
	ccs_expression_type_t *type_ret)
{
	CCS_CHECK_OBJ(expression, CCS_OBJECT_TYPE_EXPRESSION);
	CCS_CHECK_PTR(type_ret);
	*type_ret = expression->data->type;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_literal_get_value(ccs_expression_t expression, ccs_datum_t *value_ret)
{
	CCS_CHECK_OBJ(expression, CCS_OBJECT_TYPE_EXPRESSION);
	CCS_CHECK_PTR(value_ret);
	CCS_REFUTE(
		expression->data->type != CCS_EXPRESSION_TYPE_LITERAL,
		CCS_INVALID_EXPRESSION);
	_ccs_expression_literal_data_t *d =
		(_ccs_expression_literal_data_t *)expression->data;
	*value_ret = d->value;
	return CCS_SUCCESS;
}

ccs_error_t
ccs_variable_get_parameter(
	ccs_expression_t expression,
	ccs_parameter_t *parameter_ret)
{
	CCS_CHECK_OBJ(expression, CCS_OBJECT_TYPE_EXPRESSION);
	CCS_CHECK_PTR(parameter_ret);
	CCS_REFUTE(
		expression->data->type != CCS_EXPRESSION_TYPE_VARIABLE,
		CCS_INVALID_EXPRESSION);
	_ccs_expression_variable_data_t *d =
		(_ccs_expression_variable_data_t *)expression->data;
	*parameter_ret = d->parameter;
	return CCS_SUCCESS;
}

#undef utarray_oom
#define utarray_oom()                                                          \
	{                                                                      \
		CCS_RAISE(                                                     \
			CCS_OUT_OF_MEMORY,                                     \
			"Not enough memory to allocate array");                \
	}

static ccs_error_t
_get_parameters(ccs_expression_t expression, UT_array *array)
{
	CCS_CHECK_OBJ(expression, CCS_OBJECT_TYPE_EXPRESSION);
	if (expression->data->type == CCS_EXPRESSION_TYPE_VARIABLE) {
		_ccs_expression_variable_data_t *d =
			(_ccs_expression_variable_data_t *)expression->data;
		utarray_push_back(array, &(d->parameter));
	} else
		for (size_t i = 0; i < expression->data->num_nodes; i++)
			CCS_VALIDATE(_get_parameters(
				expression->data->nodes[i], array));
	return CCS_SUCCESS;
}

static const UT_icd _parameter_icd = {
	sizeof(ccs_parameter_t),
	NULL,
	NULL,
	NULL,
};

static int
_parameter_sort(const void *a, const void *b)
{
	ccs_parameter_t ha = *(ccs_parameter_t *)a;
	ccs_parameter_t hb = *(ccs_parameter_t *)b;
	return ha < hb ? -1 : ha > hb ? 1 : 0;
}

ccs_error_t
ccs_expression_get_parameters(
	ccs_expression_t expression,
	size_t           num_parameters,
	ccs_parameter_t *parameters,
	size_t          *num_parameters_ret)
{
	CCS_CHECK_OBJ(expression, CCS_OBJECT_TYPE_EXPRESSION);
	CCS_CHECK_ARY(num_parameters, parameters);
	CCS_REFUTE(!parameters && !num_parameters_ret, CCS_INVALID_VALUE);
	ccs_error_t err = CCS_SUCCESS;
	UT_array   *array;
	size_t      count = 0;
	utarray_new(array, &_parameter_icd);
	CCS_VALIDATE_ERR_GOTO(
		err, _get_parameters(expression, array), errutarray);
	utarray_sort(array, &_parameter_sort);
	if (utarray_len(array) > 0) {
		ccs_parameter_t  previous = NULL;
		ccs_parameter_t *p_h      = NULL;
		while ((p_h = (ccs_parameter_t *)utarray_next(array, p_h))) {
			if (*p_h != previous) {
				count += 1;
				previous = *p_h;
			}
		}
	} else
		count = 0;
	if (parameters) {
		CCS_REFUTE_ERR_GOTO(
			err, count > num_parameters, CCS_INVALID_VALUE,
			errutarray);
		ccs_parameter_t  previous = NULL;
		ccs_parameter_t *p_h      = NULL;
		size_t           index    = 0;
		while ((p_h = (ccs_parameter_t *)utarray_next(array, p_h))) {
			if (*p_h != previous) {
				parameters[index++] = *p_h;
				previous            = *p_h;
			}
		}
		for (size_t i = count; i < num_parameters; i++)
			parameters[i] = NULL;
	}
	if (num_parameters_ret)
		*num_parameters_ret = count;
errutarray:
	utarray_free(array);
	return err;
}

ccs_error_t
ccs_expression_check_context(ccs_expression_t expression, ccs_context_t context)
{
	CCS_CHECK_OBJ(expression, CCS_OBJECT_TYPE_EXPRESSION);
	ccs_error_t err = CCS_SUCCESS;
	UT_array   *array;
	utarray_new(array, &_parameter_icd);
	CCS_VALIDATE_ERR_GOTO(
		err, _get_parameters(expression, array), errutarray);
	utarray_sort(array, &_parameter_sort);
	if (utarray_len(array) > 0) {
		CCS_REFUTE_ERR_GOTO(
			err, !context, CCS_INVALID_VALUE, errutarray);
		ccs_parameter_t  previous = NULL;
		ccs_parameter_t *p_h      = NULL;
		while ((p_h = (ccs_parameter_t *)utarray_next(array, p_h))) {
			if (*p_h != previous) {
				size_t index;
				CCS_VALIDATE_ERR_GOTO(
					err,
					ccs_context_get_parameter_index(
						context, *p_h, &index),
					errutarray);
				previous = *p_h;
			}
		}
	}
errutarray:
	utarray_free(array);
	return err;
}
