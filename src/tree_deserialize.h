#ifndef _TREE_DESERIALIZE_H
#define _TREE_DESERIALIZE_H
#include "cconfigspace_internal.h"
#include "tree_internal.h"

struct _ccs_tree_data_mock_s {
	size_t      arity;
	ccs_float_t weight;
	ccs_tree_t *children;
	ccs_float_t bias;
	ccs_datum_t value;
};
typedef struct _ccs_tree_data_mock_s _ccs_tree_data_mock_t;

static inline ccs_error_t
_ccs_tree_deserialize(
	ccs_tree_t                        *tree_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts);

static inline ccs_error_t
_ccs_deserialize_bin_ccs_tree_data(
	_ccs_tree_data_mock_t             *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	new_opts.handle_map                        = NULL;
	CCS_VALIDATE(
		_ccs_deserialize_bin_size(&data->arity, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
		&data->weight, buffer_size, buffer));
	if (data->arity) {
		data->children =
			(ccs_tree_t *)calloc(data->arity, sizeof(ccs_tree_t));
		CCS_REFUTE(!data->children, CCS_OUT_OF_MEMORY);
		for (size_t i = 0; i < data->arity; i++) {
			ccs_bool_t present;
			CCS_VALIDATE(_ccs_deserialize_bin_ccs_bool(
				&present, buffer_size, buffer));
			if (present)
				CCS_VALIDATE(_ccs_tree_deserialize(
					data->children + i,
					CCS_SERIALIZE_FORMAT_BINARY, version,
					buffer_size, buffer, &new_opts));
		}
	}
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_float(
		&data->bias, buffer_size, buffer));
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_datum(
		&data->value, buffer_size, buffer));
	return CCS_SUCCESS;
}

static inline ccs_error_t
_ccs_deserialize_bin_tree(
	ccs_tree_t                        *tree_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_error_t            res = CCS_SUCCESS;
	_ccs_object_internal_t obj;
	ccs_object_t           handle;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(obj.type != CCS_TREE, CCS_INVALID_TYPE);

	ccs_tree_t            tree;
	_ccs_tree_data_mock_t data;
	data.children = NULL;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_tree_data(
			&data, version, buffer_size, buffer, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res, ccs_create_tree(data.arity, data.value, &tree), end);
	CCS_VALIDATE_ERR_GOTO(
		res, ccs_tree_set_weight(tree, data.weight), err_tree);
	CCS_VALIDATE_ERR_GOTO(
		res, ccs_tree_set_bias(tree, data.bias), err_tree);
	for (size_t i = 0; i < data.arity; i++)
		if (data.children[i])
			CCS_VALIDATE_ERR_GOTO(
				res,
				ccs_tree_set_child(tree, i, data.children[i]),
				err_tree);
	if (opts && opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle, (ccs_object_t)tree),
			err_tree);

	*tree_ret = tree;
	goto end;
err_tree:
	ccs_release_object(tree);
end:
	if (data.children) {
		for (size_t i = 0; i < data.arity; i++)
			if (data.children[i])
				ccs_release_object(data.children[i]);
		free(data.children);
	}
	return res;
}

static ccs_error_t
_ccs_tree_deserialize(
	ccs_tree_t                        *tree_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_tree(
			tree_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_deserialize_user_data(
		(ccs_object_t)*tree_ret, format, version, buffer_size, buffer,
		opts));
	return CCS_SUCCESS;
}

#endif //_TREE_DESERIALIZE_H
