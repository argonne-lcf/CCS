#ifndef _EXPRESSION_DESERIALIZE_H
#define _EXPRESSION_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "expression_internal.h"

struct _ccs_expression_data_mock_s {
	ccs_expression_type_t type;
	size_t                num_nodes;
	ccs_datum_t          *nodes;
};
typedef struct _ccs_expression_data_mock_s _ccs_expression_data_mock_t;

static inline ccs_error_t
_ccs_deserialize_bin_expression(
	ccs_expression_t                  *expression_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts);

static ccs_error_t
_ccs_expression_deserialize(
	ccs_expression_t                  *expression_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts);

static inline ccs_error_t
_ccs_deserialize_bin_ccs_expression_data(
	_ccs_expression_data_mock_t       *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	data->nodes = NULL;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_expression_type(
		&data->type, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_size(
		&data->num_nodes, buffer_size, buffer));
	if (data->num_nodes) {
		data->nodes = (ccs_datum_t *)calloc(
			data->num_nodes, sizeof(ccs_datum_t));
		CCS_REFUTE(!data->nodes, CCS_OUT_OF_MEMORY);
		for (size_t i = 0; i < data->num_nodes; i++) {
			ccs_expression_t expr;
			CCS_VALIDATE(_ccs_expression_deserialize(
				&expr, CCS_SERIALIZE_FORMAT_BINARY, version,
				buffer_size, buffer, opts));
			data->nodes[i].type    = CCS_OBJECT;
			data->nodes[i].value.o = expr;
		}
	}
	return CCS_SUCCESS;
}

struct _ccs_expression_literal_data_mock_s {
	_ccs_expression_data_mock_t expr;
	ccs_datum_t                 value;
};
typedef struct _ccs_expression_literal_data_mock_s
	_ccs_expression_literal_data_mock_t;

static inline ccs_error_t
_ccs_deserialize_bin_ccs_expression_literal_data(
	_ccs_expression_literal_data_mock_t *data,
	uint32_t                             version,
	size_t                              *buffer_size,
	const char                         **buffer)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_expression_data(
		&data->expr, version, buffer_size, buffer, NULL));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(
		&data->value, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_expression_literal(
	ccs_expression_t *expression_ret,
	uint32_t          version,
	size_t           *buffer_size,
	const char      **buffer)
{
	_ccs_expression_literal_data_mock_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_expression_literal_data(
		&data, version, buffer_size, buffer));
	CCS_VALIDATE(ccs_create_literal(data.value, expression_ret));
	return CCS_SUCCESS;
}

struct _ccs_expression_variable_data_mock_s {
	_ccs_expression_data_mock_t expr;
	ccs_parameter_t             parameter;
};
typedef struct _ccs_expression_variable_data_mock_s
	_ccs_expression_variable_data_mock_t;

static inline ccs_error_t
_ccs_deserialize_bin_ccs_expression_variable_data(
	_ccs_expression_variable_data_mock_t *data,
	uint32_t                              version,
	size_t                               *buffer_size,
	const char                          **buffer)
{
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_expression_data(
		&data->expr, version, buffer_size, buffer, NULL));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object(
		(ccs_object_t *)&data->parameter, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_expression_variable(
	ccs_expression_t                  *expression_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	CCS_CHECK_OBJ(opts->handle_map, CCS_MAP);
	_ccs_expression_variable_data_mock_t data;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_expression_variable_data(
		&data, version, buffer_size, buffer));
	ccs_datum_t     d;
	ccs_parameter_t h;
	CCS_VALIDATE(
		ccs_map_get(opts->handle_map, ccs_object(data.parameter), &d));
	CCS_REFUTE(d.type != CCS_OBJECT, CCS_INVALID_HANDLE);
	h = (ccs_parameter_t)(d.value.o);
	CCS_VALIDATE(ccs_create_variable(h, expression_ret));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_expression_general(
	ccs_expression_t                  *expression_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_error_t                 res = CCS_SUCCESS;
	_ccs_expression_data_mock_t data;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_expression_data(
			&data, version, buffer_size, buffer, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_expression(
			data.type, data.num_nodes, data.nodes, expression_ret),
		end);
end:
	if (data.nodes) {
		for (size_t i = 0; i < data.num_nodes; i++)
			if (data.nodes[i].type == CCS_OBJECT)
				ccs_release_object(data.nodes[i].value.o);
		free(data.nodes);
	}
	return res;
}

static inline ccs_error_t
_ccs_deserialize_bin_expression(
	ccs_expression_t                  *expression_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	new_opts.map_values                        = CCS_FALSE;
	_ccs_object_internal_t obj;
	ccs_object_t           handle;
	ccs_error_t            res;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(obj.type != CCS_EXPRESSION, CCS_INVALID_TYPE);

	ccs_expression_type_t dtype;
	CCS_VALIDATE(
		_ccs_peek_bin_ccs_expression_type(&dtype, buffer_size, buffer));
	switch (dtype) {
	case CCS_LITERAL:
		CCS_VALIDATE(_ccs_deserialize_bin_expression_literal(
			expression_ret, version, buffer_size, buffer));
		break;
	case CCS_VARIABLE:
		CCS_VALIDATE(_ccs_deserialize_bin_expression_variable(
			expression_ret, version, buffer_size, buffer,
			&new_opts));
		break;
	default:
		CCS_REFUTE(
			dtype < CCS_OR || dtype >= CCS_EXPRESSION_TYPE_MAX,
			CCS_UNSUPPORTED_OPERATION);
		CCS_VALIDATE(_ccs_deserialize_bin_expression_general(
			expression_ret, version, buffer_size, buffer,
			&new_opts));
	}
	if (opts && opts->map_values && opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*expression_ret),
			err_exp);

	return CCS_SUCCESS;
err_exp:
	ccs_release_object(*expression_ret);
	return res;
}

static ccs_error_t
_ccs_expression_deserialize(
	ccs_expression_t                  *expression_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_expression(
			expression_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_deserialize_user_data(
		(ccs_object_t)*expression_ret, format, version, buffer_size,
		buffer, opts));
	return CCS_SUCCESS;
}

#endif //_EXPRESSION_DESERIALIZE_H
