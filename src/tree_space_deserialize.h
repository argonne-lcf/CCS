#ifndef _TREE_SPACE_DESERIALIZE_H
#define _TREE_SPACE_DESERIALIZE_H
#include "tree_space_internal.h"

static inline ccs_result_t
_ccs_deserialize_bin_ccs_tree_space_common_data(
	_ccs_tree_space_common_data_t     *data,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_object_deserialize_options_t new_opts = *opts;
	new_opts.handle_map                        = NULL;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_tree_space_type(
		&data->type, buffer_size, buffer));
	CCS_VALIDATE(
		_ccs_deserialize_bin_string(&data->name, buffer_size, buffer));
	CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
		(ccs_object_t *)&data->rng, CCS_OBJECT_TYPE_RNG,
		CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer,
		&new_opts));
	CCS_VALIDATE(_ccs_object_deserialize_with_opts_check(
		(ccs_object_t *)&data->tree, CCS_OBJECT_TYPE_TREE,
		CCS_SERIALIZE_FORMAT_BINARY, version, buffer_size, buffer,
		&new_opts));
	return CCS_RESULT_SUCCESS;
}

static inline ccs_result_t
_ccs_deserialize_bin_tree_space_static(
	ccs_tree_space_t                  *tree_space_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t                  res  = CCS_RESULT_SUCCESS;
	_ccs_tree_space_common_data_t data = {
		CCS_TREE_SPACE_TYPE_STATIC, NULL, NULL, NULL};
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_tree_space_common_data(
			&data, version, buffer_size, buffer, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_static_tree_space(
			data.name, data.tree, tree_space_ret),
		end);
end:
	if (data.rng)
		ccs_release_object(data.rng);
	if (data.tree)
		ccs_release_object(data.tree);
	return res;
}

struct _ccs_tree_space_dynamic_data_mock_s {
	_ccs_tree_space_common_data_t common_data;
	_ccs_blob_t                   blob;
};
typedef struct _ccs_tree_space_dynamic_data_mock_s
	_ccs_tree_space_dynamic_data_mock_t;

static inline ccs_result_t
_ccs_deserialize_bin_tree_space_dynamic(
	ccs_tree_space_t                  *tree_space_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	_ccs_tree_space_dynamic_data_mock_t data = {
		{CCS_TREE_SPACE_TYPE_DYNAMIC, NULL, NULL, NULL}, {0, NULL}};
	ccs_dynamic_tree_space_vector_t *vector =
		(ccs_dynamic_tree_space_vector_t *)opts->vector;
	ccs_result_t res = CCS_RESULT_SUCCESS;
	CCS_VALIDATE_ERR_GOTO(
		res,
		_ccs_deserialize_bin_ccs_tree_space_common_data(
			&data.common_data, version, buffer_size, buffer, opts),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_create_dynamic_tree_space(
			data.common_data.name, data.common_data.tree, vector,
			opts->data, tree_space_ret),
		end);
	CCS_VALIDATE_ERR_GOTO(
		res,
		ccs_tree_space_set_rng(*tree_space_ret, data.common_data.rng),
		tree_space);
	if (vector->deserialize_state)
		CCS_VALIDATE_ERR_GOTO(
			res,
			vector->deserialize_state(
				*tree_space_ret, data.blob.sz, data.blob.blob),
			tree_space);
	goto end;
tree_space:
	ccs_release_object(*tree_space_ret);
	*tree_space_ret = NULL;
end:
	if (data.common_data.rng)
		ccs_release_object(data.common_data.rng);
	if (data.common_data.tree)
		ccs_release_object(data.common_data.tree);
	return res;
}

static inline ccs_result_t
_ccs_deserialize_bin_tree_space(
	ccs_tree_space_t                  *tree_space_ret,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	ccs_result_t           res = CCS_RESULT_SUCCESS;
	_ccs_object_internal_t obj;
	ccs_object_t           handle;
	CCS_VALIDATE(_ccs_deserialize_bin_ccs_object_internal(
		&obj, buffer_size, buffer, &handle));
	CCS_REFUTE(
		obj.type != CCS_OBJECT_TYPE_TREE_SPACE,
		CCS_RESULT_ERROR_INVALID_TYPE);

	ccs_tree_space_type_t stype;
	CCS_VALIDATE(
		_ccs_peek_bin_ccs_tree_space_type(&stype, buffer_size, buffer));
	switch (stype) {
	case CCS_TREE_SPACE_TYPE_STATIC:
		CCS_VALIDATE(_ccs_deserialize_bin_tree_space_static(
			tree_space_ret, version, buffer_size, buffer, opts));
		break;
	case CCS_TREE_SPACE_TYPE_DYNAMIC:
		CCS_VALIDATE(_ccs_deserialize_bin_tree_space_dynamic(
			tree_space_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_UNSUPPORTED_OPERATION,
			"Unsupported tree space type: %d", stype);
	}
	if (opts && opts->handle_map)
		CCS_VALIDATE_ERR_GOTO(
			res,
			_ccs_object_handle_check_add(
				opts->handle_map, handle,
				(ccs_object_t)*tree_space_ret),
			err_tree_space);
	return CCS_RESULT_SUCCESS;
err_tree_space:
	ccs_release_object(*tree_space_ret);
	*tree_space_ret = NULL;
	return res;
}

static ccs_result_t
_ccs_tree_space_deserialize(
	ccs_tree_space_t                  *tree_space_ret,
	ccs_serialize_format_t             format,
	uint32_t                           version,
	size_t                            *buffer_size,
	const char                       **buffer,
	_ccs_object_deserialize_options_t *opts)
{
	switch (format) {
	case CCS_SERIALIZE_FORMAT_BINARY:
		CCS_VALIDATE(_ccs_deserialize_bin_tree_space(
			tree_space_ret, version, buffer_size, buffer, opts));
		break;
	default:
		CCS_RAISE(
			CCS_RESULT_ERROR_INVALID_VALUE,
			"Unsupported serialization format: %d", format);
	}
	CCS_VALIDATE(_ccs_object_deserialize_user_data(
		(ccs_object_t)*tree_space_ret, format, version, buffer_size,
		buffer, opts));
	return CCS_RESULT_SUCCESS;
}

#endif //_TREE_SPACE_DESERIALIZE_H
